#include "slim.h"

#include <math.h>
#include <stdarg.h>

// Internal Routines ---------------------------------------------------------------------------------------------------

SlimBytecode* slim_bytecode_load(const char* filename) {
    // Open the file

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        slim_error("Failed to open file\n");
        return NULL;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    u32_t size = ftell(file);
    if (size % 9 != 0) {
        slim_error("Invalid bytecode file\n");
        return NULL;
    }

    // Allocate memory for the bytecode
    u8_t* data = malloc(size);
    if (data == NULL) {
        slim_error("Failed to allocate memory for bytecode\n");
        return NULL;
    }

    // Read the file into memory
    fseek(file, 0, SEEK_SET);
    fread(data, size, 1, file);
    fclose(file);

    // Create the bytecode struct
    SlimBytecode* bytecode = malloc(sizeof(SlimBytecode));
    bytecode->data = data;
    bytecode->size = size / 9;
    bytecode->bytesize = size;

    return bytecode;
}

void slim_bytecode_destroy(SlimBytecode* bytecode) {
    free(bytecode->data);
    free(bytecode);
}

SlimError ___slim_machine_push(SlimMachine* machine, u64_t value) {
    if (machine->operand_stack_pointer >= SLIM_MACHINE_OPERAND_STACK_SIZE) {
        return SL_ERROR_STACK_OVERFLOW;
    }

    machine->operand_stack[machine->operand_stack_pointer++] = value;

    return SL_ERROR_NONE;
}

SlimError ___slim_machine_pop(SlimMachine* machine, u64_t* value) {
    if (machine->operand_stack_pointer == 0) {
        return SL_ERROR_STACK_UNDERFLOW;
    }

    u64_t top = machine->operand_stack[machine->operand_stack_pointer - 1];
    machine->operand_stack[machine->operand_stack_pointer - 1] = 0;
    machine->operand_stack_pointer--;
    *value = top;

    return SL_ERROR_NONE;
}

SlimError ___slim_machine_load(SlimMachine* machine, u32_t index) {
    if (index >= SLIM_MACHINE_REGISTERS) {
        return SL_ERROR_INVALID_REGISTER;
    }

    u64_t value = machine->registers[index];
    SlimError error = ___slim_machine_push(machine, value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    return SL_ERROR_NONE;
}

SlimError ___slim_machine_store(SlimMachine* machine, u32_t index) {
    if (index >= SLIM_MACHINE_REGISTERS) {
        return SL_ERROR_INVALID_REGISTER;
    }

    u64_t value;
    SlimError error = ___slim_machine_pop(machine, &value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    machine->registers[index] = value;

    return SL_ERROR_NONE;
}

SlimError ___slim_machine_read(SlimMachine* machine, u32_t address, u32_t offset) {
    u64_t value;
    SlimError error;

    u64_t* ptr = (u64_t*)(machine->memory + address + offset);
    value = *ptr;

    error = ___slim_machine_push(machine, value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    return SL_ERROR_NONE;
}

SlimError ___slim_machine_write(SlimMachine* machine, u32_t address, u32_t offset) {
    u64_t value;
    SlimError error;

    error = ___slim_machine_pop(machine, &value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    u64_t* ptr = (u64_t*)(machine->memory + address + offset);
    *ptr = value;

    return SL_ERROR_NONE;
}

// TODO: Validate Me
SlimError slim_machine_alloc(SlimMachine* machine, u32_t size, u32_t* address) {
    SlimBlock* block = machine->blocks;
    while (block != NULL) {
        if (block->allocated == 0 && block->end - block->start >= size) {
            SlimError error = slim_block_split(block, size);
            if (error != SL_ERROR_NONE) {
                return error;
            }

            block->allocated = 1;
            *address = block->start;
            return SL_ERROR_NONE;
        }

        block = block->next;
    }

    return SL_ERROR_BLOCK_ALLOC;
}

// TODO: Validate Me
SlimError ___slim_machine_free(SlimMachine* machine, u32_t address) {
    SlimBlock* block = machine->blocks;
    while (block != NULL) {
        if (block->start == address) {
            block->allocated = 0;
            SlimError error = slim_block_merge(block);
            if (error != SL_ERROR_NONE) {
                return error;
            }

            return SL_ERROR_NONE;
        }

        block = block->next;
    }

    return SL_ERROR_BLOCK_FREE;
}

SlimError ___slim_machine_call(SlimMachine* machine, u32_t address) {
    machine->call_stack_pointer++;
    if (machine->call_stack_pointer >= SLIM_MACHINE_CALL_STACK_SIZE) {
        return SL_ERROR_STACK_OVERFLOW;
    }

    machine->call_stack[machine->call_stack_pointer].instruction_pointer = machine->instruction_pointer;
    machine->call_stack[machine->call_stack_pointer].size = 0;
    machine->instruction_pointer = address;
    return SL_ERROR_NONE;
}

SlimError ___slim_machine_ret(SlimMachine* machine) {
    u32_t ret_address = machine->call_stack[machine->call_stack_pointer].instruction_pointer;
    u16_t count = machine->call_stack[machine->call_stack_pointer].size;
    machine->call_stack[machine->call_stack_pointer].instruction_pointer = 0;
    machine->call_stack[machine->call_stack_pointer].size = 0;

    machine->call_stack_pointer--;
    if (machine->call_stack_pointer < 0) {
        return SL_ERROR_STACK_UNDERFLOW;
    }

    machine->instruction_pointer = ret_address;

    for (u16_t i = 0; i < count; i++) {
        u64_t value;
        SlimError error = ___slim_machine_pop(machine, &value);
        if (error != SL_ERROR_NONE) {
            return error;
        }
    }

    return SL_ERROR_NONE;
}
// Routines and Operations ---------------------------------------------------------------------------------------------

void slim_routine_nop(SlimMachine* machine, SlimInstruction instruction) { slim_info("[ROUTINE]\tNOP\n") return; }

void slim_routine_halt(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tHALT\n") machine->flags.halt = 1;
    return;
}

void slim_routine_loadi(SlimMachine* machine, SlimInstruction instruction) {
    SlimError error;

    u64_t value = (u64_t)instruction.arg1 << 32 | instruction.arg2;
    slim_info("[ROUTINE]\tLOADI %lld\n", value)

        error = ___slim_machine_push(machine, value);

    if (error != SL_ERROR_NONE) {
        printf("ERROR: %d\n", error);
    }

    slim_machine_except(machine, error);
}

void slim_routine_loadr(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tLOADR %d\n", instruction.arg1)

        u32_t index = instruction.arg1;

    SlimError error = ___slim_machine_load(machine, index);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_loadm(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tLOADM %d, %d\n", instruction.arg1, instruction.arg2)

        u64_t address = 0;
    // TODO: Which arg is it?
    u32_t offset = instruction.arg1;
    SlimError error;

    error = ___slim_machine_pop(machine, &address);
    slim_machine_except(machine, error);

    error = ___slim_machine_read(machine, (u32_t)address, offset);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_drop(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tDROP\n")

        SlimError error = ___slim_machine_pop(machine, NULL);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_storer(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tSTORER %d\n", instruction.arg1)

        u32_t index = instruction.arg1;
    SlimError error;

    error = ___slim_machine_store(machine, index);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_storem(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tSTOREM %d\n", instruction.arg1);

    u64_t address;
    u64_t offset;
    SlimError error;

    error = ___slim_machine_pop(machine, &address);
    slim_machine_except(machine, error);

    slim_machine_except(machine, error);

    offset = instruction.arg1;
    error = ___slim_machine_write(machine, address, offset);

    return;
}

void slim_routine_dup(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tDUP\n");

    u64_t value;
    SlimError error;

    error = ___slim_machine_pop(machine, &value);
    slim_machine_except(machine, error);

    error = ___slim_machine_push(machine, value);
    slim_machine_except(machine, error);

    error = ___slim_machine_push(machine, value);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_swap(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tSWAP\n");

    u64_t a;
    u64_t b;
    SlimError error;

    error = ___slim_machine_pop(machine, &a);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop(machine, &b);
    slim_machine_except(machine, error);

    error = ___slim_machine_push(machine, a);
    slim_machine_except(machine, error);

    error = ___slim_machine_push(machine, b);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_rot(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tROT\n");

    u64_t a;
    u64_t b;
    u64_t c;
    SlimError error;

    error = ___slim_machine_pop(machine, &a);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop(machine, &b);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop(machine, &c);
    slim_machine_except(machine, error);

    error = ___slim_machine_push(machine, b);
    slim_machine_except(machine, error);

    error = ___slim_machine_push(machine, a);
    slim_machine_except(machine, error);

    error = ___slim_machine_push(machine, c);
    slim_machine_except(machine, error);

    return;
}

#define ___slim_routine_binary_integer(operation)                                                                      \
    u64_t a;                                                                                                           \
    u64_t b;                                                                                                           \
    SlimError error;                                                                                                   \
    error = ___slim_machine_pop(machine, &b);                                                                          \
    slim_machine_except(machine, error);                                                                               \
    error = ___slim_machine_pop(machine, &a);                                                                          \
    slim_machine_except(machine, error);                                                                               \
    u64_t result = a operation b;                                                                                      \
    error = ___slim_machine_push(machine, result);                                                                     \
    slim_machine_except(machine, error);

#define ___slim_routine_binary_float(operation)                                                                        \
    u64_t a;                                                                                                           \
    u64_t b;                                                                                                           \
    SlimError error;                                                                                                   \
    error = ___slim_machine_pop(machine, &b);                                                                          \
    slim_machine_except(machine, error);                                                                               \
    error = ___slim_machine_pop(machine, &a);                                                                          \
    slim_machine_except(machine, error);                                                                               \
    double a_double = *((double*)&a);                                                                                  \
    double b_double = *((double*)&b);                                                                                  \
    double result_double = a_double operation b_double;                                                                \
    u64_t result = *((u64_t*)&result_double);                                                                          \
    error = ___slim_machine_push(machine, result);                                                                     \
    slim_machine_except(machine, error);

void slim_routine_add(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tADD\n");
    ___slim_routine_binary_integer(+);
}

void slim_routine_sub(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tSUB\n");
    ___slim_routine_binary_integer(-);
}

void slim_routine_mul(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tMUL\n");
    ___slim_routine_binary_integer(*);
}

void slim_routine_div(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tDIV\n");
    ___slim_routine_binary_integer(/);
}

void slim_routine_mod(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tMOD\n");
    ___slim_routine_binary_integer(%);
}

void slim_routine_addf(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tADDF\n");
    ___slim_routine_binary_float(+);
}

void slim_routine_subf(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tSUBF\n");
    ___slim_routine_binary_float(-);
}

void slim_routine_mulf(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tMULF\n");
    ___slim_routine_binary_float(*);
}

void slim_routine_divf(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tDIVF\n");
    ___slim_routine_binary_float(/);
}

void slim_routine_modf(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tMODF\n");

    u64_t a;
    u64_t b;
    SlimError error;

    error = ___slim_machine_pop(machine, &b);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop(machine, &a);
    slim_machine_except(machine, error);

    double a_double = *((double*)&a);
    double b_double = *((double*)&b);
    double result_double = fmod(a_double, b_double);
    u64_t result = *((u64_t*)&result_double);

    error = ___slim_machine_push(machine, result);
    slim_machine_except(machine, error);
}

void slim_routine_alloc(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tALLOC %d\n", instruction.arg1);

    u32_t size = instruction.arg1;
    SlimError error;

    u32_t address;

    error = slim_machine_alloc(machine, size, &address);
    slim_machine_except(machine, error);

    error = ___slim_machine_push(machine, address);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_free(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tFREE %d\n", instruction.arg1);

    SlimError error = ___slim_machine_free(machine, instruction.arg1);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_jmp(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tJUMP %d\n", instruction.arg2);

    u32_t address = instruction.arg2;
    machine->instruction_pointer = address;
}

void slim_routine_jne(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tJNE %d\n", instruction.arg1);

    u64_t value;
    SlimError error = ___slim_machine_pop(machine, &value);
    slim_machine_except(machine, error);

    if (value != 0) {
        machine->instruction_pointer = instruction.arg1;
    }
}

void slim_routine_je(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tJE %d\n", instruction.arg1);

    u64_t value;
    SlimError error = ___slim_machine_pop(machine, &value);
    slim_machine_except(machine, error);

    if (value == 0) {
        machine->instruction_pointer = instruction.arg1;
    }
}

void slim_routine_call(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tCALL %x\n", instruction.arg2);

    u32_t address = instruction.arg2;
    SlimError error = ___slim_machine_call(machine, address);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_ret(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tRET\n");

    SlimError error = ___slim_machine_ret(machine);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_ftoi(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tFTOI\n");

    u64_t value;
    SlimError error;

    error = ___slim_machine_pop(machine, &value);
    slim_machine_except(machine, error);

    double value_double = *((double*)&value);
    u64_t result = (u64_t)value_double;

    error = ___slim_machine_push(machine, result);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_itof(SlimMachine* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tITOF\n");

    u64_t value;
    SlimError error;

    error = ___slim_machine_pop(machine, &value);
    slim_machine_except(machine, error);

    double result_double = (double)value;
    u64_t result = *((u64_t*)&result_double);

    error = ___slim_machine_push(machine, result);
    slim_machine_except(machine, error);

    return;
}

// Fetch, Decode, Execute ----------------------------------------------------------------------------------------------
SlimInstruction slim_machine_fetch(SlimMachine* machine) {
    SlimInstruction instruction;
    instruction.opcode = machine->bytecode[machine->instruction_pointer];

    // Read in the next 4 bytes as the first argument
    u32_t arg1 = 0;
    for (int i = 0; i < 4; i++) {
        arg1 |= machine->bytecode[machine->instruction_pointer + 1 + i] << (i * 8);
    }

    // WARN: This is a hack to get around endianness issues
    // Reverse the bits
    arg1 = ((arg1 & 0x000000FF) << 24) | ((arg1 & 0x0000FF00) << 8) | ((arg1 & 0x00FF0000) >> 8) |
           ((arg1 & 0xFF000000) >> 24);

    // Read in the next 4 bytes as the second argument
    u32_t arg2 = 0;
    for (int i = 0; i < 4; i++) {
        arg2 |= machine->bytecode[machine->instruction_pointer + 5 + i] << (i * 8);
    }

    // WARN: This is a hack to get around endianness issues
    // Reverse the bits
    arg2 = ((arg2 & 0x000000FF) << 24) | ((arg2 & 0x0000FF00) << 8) | ((arg2 & 0x00FF0000) >> 8) |
           ((arg2 & 0xFF000000) >> 24);

    instruction.arg1 = arg1;
    instruction.arg2 = arg2;

    slim_info("[FETCH]\t0x%x 0x%x 0x%x\n", instruction.opcode, instruction.arg1, instruction.arg2);

    machine->instruction_pointer += 9;

    return instruction;
}

SlimRoutine slim_machine_decode(SlimMachine* machine, SlimInstruction instruction) {
    switch (instruction.opcode) {
    case SL_OPCODE_NOOP: return slim_routine_nop; break;
    case SL_OPCODE_HALT: return slim_routine_halt; break;
    case SL_OPCODE_LOADI: return slim_routine_loadi; break;
    case SL_OPCODE_LOADR: return slim_routine_loadr; break;
    case SL_OPCODE_LOADM: return slim_routine_loadm; break;
    case SL_OPCODE_DROP: return slim_routine_drop; break;
    case SL_OPCODE_STORER: return slim_routine_storer; break;
    case SL_OPCODE_STOREM: return slim_routine_storem; break;
    case SL_OPCODE_DUP: return slim_routine_dup; break;
    case SL_OPCODE_SWAP: return slim_routine_swap; break;
    case SL_OPCODE_ROT: return slim_routine_rot; break;
    case SL_OPCODE_ADD: return slim_routine_add; break;
    case SL_OPCODE_SUB: return slim_routine_sub; break;
    case SL_OPCODE_MUL: return slim_routine_mul; break;
    case SL_OPCODE_DIV: return slim_routine_div; break;
    case SL_OPCODE_MOD: return slim_routine_mod; break;
    case SL_OPCODE_ADDF: return slim_routine_addf; break;
    case SL_OPCODE_SUBF: return slim_routine_subf; break;
    case SL_OPCODE_MULF: return slim_routine_mulf; break;
    case SL_OPCODE_DIVF: return slim_routine_divf; break;
    case SL_OPCODE_MODF: return slim_routine_modf; break;
    case SL_OPCODE_ALLOC: return slim_routine_alloc; break;
    case SL_OPCODE_FREE: return slim_routine_free; break;
    case SL_OPCODE_JMP: return slim_routine_jmp; break;
    case SL_OPCODE_JNE: return slim_routine_jne; break;
    case SL_OPCODE_JE: return slim_routine_je; break;
    case SL_OPCODE_CALL: return slim_routine_call; break;
    case SL_OPCODE_RET: return slim_routine_ret; break;
    case SL_OPCODE_ITOF: return slim_routine_itof; break;
    case SL_OPCODE_FTOI: return slim_routine_ftoi; break;
    default: return NULL; break;
    }
}

void slim_machine_execute(SlimMachine* machine, SlimRoutine routine, SlimInstruction instruction) {
    if (routine) {
        routine(machine, instruction);
    } else {
        slim_error("[EXECUTE]\tInvalid instruction 0x%x\n", instruction.opcode);
        machine->flags.error = 1;
    }
}

// External API --------------------------------------------------------------------------------------------------------
SlimMachine* slim_machine_create() {
    SlimMachine* machine = malloc(sizeof(SlimMachine));
    machine->bytecode = NULL;
    machine->bytecode_size = 0;
    machine->blocks = slim_block_create(0, SLIM_MACHINE_MEMORY_SIZE);
    return machine;
}

void slim_machine_destroy(SlimMachine* machine) {
    if (machine->bytecode) {
        free(machine->bytecode);
    }

    slim_block_destroy(machine->blocks);

    free(machine);
}

void slim_machine_clear(SlimMachine* machine) {
    for (u32_t i = 0; i < SLIM_MACHINE_OPERAND_STACK_SIZE; i++) {
        machine->operand_stack[i] = 0;
    }

    for (u32_t i = 0; i < SLIM_MACHINE_CALL_STACK_SIZE; i++) {
        machine->call_stack[i].instruction_pointer = 0;
        machine->call_stack[i].size = 0;
    }

    for (u32_t i = 0; i < SLIM_MACHINE_REGISTERS; i++) {
        machine->registers[i] = 0;
    }

    for (u32_t i = 0; i < SLIM_MACHINE_MEMORY_SIZE; i++) {
        machine->memory[i] = 0;
    }

    // Reset Flags
    machine->flags.zero = 0;
    machine->flags.carry = 0;
    machine->flags.overflow = 0;
    machine->flags.negative = 0;
    machine->flags.interrupt = 0;
    machine->flags.decimal = 0;
    machine->flags.error = 0;
    machine->flags.halt = 0;

    // Reset Pointers
    machine->operand_stack_pointer = 0;
    machine->call_stack_pointer = 0;
    machine->instruction_pointer = 0;

    // Reset Blocks
    slim_block_destroy(machine->blocks);
    machine->blocks = slim_block_create(0, SLIM_MACHINE_MEMORY_SIZE);
}

void slim_machine_load(SlimMachine* machine, u8_t* data, u32_t size) {
    machine->bytecode = data;
    machine->bytecode_size = size;
}

void slim_machine_launch(SlimMachine* machine) {
    while (machine->flags.halt == 0) {
        SlimInstruction instruction = slim_machine_fetch(machine);
        SlimRoutine routine = slim_machine_decode(machine, instruction);
        slim_machine_execute(machine, routine, instruction);

        if (machine->flags.error) {
            slim_error("[LAUNCH]\tError encountered, halting machine\n");
        }
    }
    slim_info("[LAUNCH]\tMachine halted\n");
}
// Block Management ----------------------------------------------------------------------------------------------------
SlimBlock* slim_block_create(u32_t start, u32_t end) {
    SlimBlock* block = malloc(sizeof(SlimBlock));
    if (block == NULL) {
        return NULL;
    }
    block->allocated = 0;
    block->start = start;
    block->end = end;
    block->next = NULL;
    return block;
}

void slim_block_destroy(SlimBlock* block) {
    if (block->next != NULL) {
        slim_block_destroy(block->next);
    }
    free(block);
}

SlimError slim_block_split(SlimBlock* block, u32_t size) {
    if (block->allocated) {
        return SL_ERROR_BLOCK_SPLIT;
    }

    if (block->end - block->start < size) {
        return SL_ERROR_BLOCK_SPLIT;
    }

    SlimBlock* new_block = slim_block_create(block->start + size, block->end);
    if (new_block == NULL) {
        return SL_ERROR_BLOCK_SPLIT;
    }

    block->end = block->start + size;
    new_block->next = block->next;
    block->next = new_block;
    return SL_ERROR_NONE;
}

SlimError slim_block_merge(SlimBlock* block) {
    if (block->allocated) {
        return SL_ERROR_BLOCK_MERGE;
    }

    if (block->next == NULL) {
        return SL_ERROR_BLOCK_MERGE;
    }

    if (block->next->allocated) {
        return SL_ERROR_BLOCK_MERGE;
    }

    block->end = block->next->end;
    SlimBlock* next = block->next;
    block->next = next->next;
    free(next);

    return SL_ERROR_NONE;
}
// Debugging -----------------------------------------------------------------------------------------------------------
SlimDebugContext* slim_debug_context;

void slim_debug_init(const char* output_path, u8_t writes_stdout) {
    slim_debug_context = malloc(sizeof(SlimDebugContext));
    slim_debug_context->output_path = output_path;
    slim_debug_context->buffer_index = 0;
    slim_debug_context->writes_stdout = writes_stdout;
}

void slim_debug_flush() {
    FILE* file = fopen(slim_debug_context->output_path, "a");
    fwrite(slim_debug_context->buffer, 1, slim_debug_context->buffer_index, file);
    fclose(file);
    slim_debug_context->buffer_index = 0;
}

void slim_debug_close() {
    slim_debug_flush();
    free(slim_debug_context);
}

void slim_debug_printf(const char* format, ...) {
    if (slim_debug_context == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    slim_debug_context->buffer_index +=
        vsprintf(slim_debug_context->buffer + slim_debug_context->buffer_index, format, args);
    va_end(args);

    if (slim_debug_context->writes_stdout) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

    if (slim_debug_context->buffer_index > SLIM_DEBUG_BUFFER_SIZE) {
        slim_debug_flush();
    }
}

void slim_debug_hexdump(void* data, u32_t length, u32_t stride, u8_t is_sparse) {
    slim_debug_printf("\nHexdump (%d bytes):\n", length);

    // Print Header Row
    slim_debug_printf("ROW    \tBYTE\t");
    for (u8_t i = 0; i < stride; i++) {
        slim_debug_printf("0%x ", i);
    }
    slim_debug_printf("\n");

    // Print the Contents
    for (int row = 0; row < length; row++) {
        // Print Index and Byte Offset
        slim_debug_printf("%04d\t", row);
        slim_debug_printf("0x%04hhX\t", row * stride);

        // Print the Bytes
        for (int byte = stride - 1; byte >= 0; byte--) {
            u8_t value = ((u8_t*)data)[row * stride + byte];
            if (is_sparse && value == 0) {
                slim_debug_printf(".. ");
            } else {
                slim_debug_printf("%x ", value);
            }
        }

        slim_debug_printf("\n");
    }
    slim_debug_printf("\n");
}