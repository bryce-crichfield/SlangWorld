#include "slim.h"

// Internal Routines ---------------------------------------------------------------------------------------------------

SlimBytecode* slim_bytecode_load(const char* filename) {
    // Open the file

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Failed to open file\n");
        return NULL;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    u32_t size = ftell(file);
    if (size % 9 != 0) {
        printf("Invalid bytecode file\n");
        return NULL;
    }

    // Allocate memory for the bytecode
    u8_t* data = malloc(size);
    if (data == NULL) {
        printf("Failed to allocate memory\n");
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
    if (machine->stack_pointer >= SLIM_MACHINE_STACK_SIZE) {
        return SL_ERROR_STACK_OVERFLOW;
    }

    machine->stack[machine->stack_pointer++] = value;

    return SL_ERROR_NONE;
}

SlimError ___slim_machine_pop(SlimMachine* machine, u64_t* value) {
    if (machine->stack_pointer == 0) {
        return SL_ERROR_STACK_UNDERFLOW;
    }

    u64_t top = machine->stack[machine->stack_pointer - 1];
    machine->stack[machine->stack_pointer - 1] = 0;
    machine->stack_pointer--;
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
// Routines and Operations ---------------------------------------------------------------------------------------------

void slim_routine_nop(SlimMachine* machine, SlimInstruction instruction) {
    printf("NOP\n");
    return;
}

void slim_routine_halt(SlimMachine* machine, SlimInstruction instruction) {
    printf("HALT\n");
    machine->flags.halt = 1;
    return;
}

void slim_routine_loadi(SlimMachine* machine, SlimInstruction instruction) {
    SlimError error;

    u64_t value = (u64_t)instruction.arg1 << 32 | instruction.arg2;
    printf("LOADI %lld\n", value);

    error = ___slim_machine_push(machine, value);

    if (error != SL_ERROR_NONE) {
        printf("ERROR: %d\n", error);
    }

    slim_machine_except(machine, error);
}

void slim_routine_loadr(SlimMachine* machine, SlimInstruction instruction) {
    printf("LOADR %d\n", instruction.arg1);

    u32_t index = instruction.arg1;

    SlimError error = ___slim_machine_load(machine, index);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_loadm(SlimMachine* machine, SlimInstruction instruction) {
    printf("LOADM %x, %x\n", instruction.arg1, instruction.arg2);

    u64_t address = 0;
    // TODO: Which arg is it?
    u32_t offset = instruction.arg1; 
    SlimError error;

    error = ___slim_machine_pop(machine, &address);
    slim_machine_except(machine, error);

    error = ___slim_machine_read(machine, (u32_t) address, offset);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_drop(SlimMachine* machine, SlimInstruction instruction) {
    printf("DROP\n");

    SlimError error = ___slim_machine_pop(machine, NULL);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_storer(SlimMachine* machine, SlimInstruction instruction) {
    printf("STORER %d\n", instruction.arg1);

    u32_t index = instruction.arg1;
    SlimError error;

    error = ___slim_machine_store(machine, index);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_storem(SlimMachine* machine, SlimInstruction instruction) {
    printf("STOREM %d\n", instruction.arg1);

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
    printf("DUP\n");

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
    printf("SWAP\n");
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
    printf("ROT\n");

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

void slim_routine_add(SlimMachine* machine, SlimInstruction instruction) {
    printf("ADD\n");

    u64_t a;
    u64_t b;
    SlimError error;

    error = ___slim_machine_pop(machine, &a);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop(machine, &b);
    slim_machine_except(machine, error);

    u64_t result = a + b;

    error = ___slim_machine_push(machine, result);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_sub(SlimMachine* machine, SlimInstruction instruction) {
    printf("SUB\n");

    u64_t a;
    u64_t b;
    SlimError error;

    error = ___slim_machine_pop(machine, &a);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop(machine, &b);
    slim_machine_except(machine, error);

    u64_t result = a - b;

    error = ___slim_machine_push(machine, result);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_mul(SlimMachine* machine, SlimInstruction instruction) {
    printf("MUL\n");

    u64_t a;
    u64_t b;
    SlimError error;

    error = ___slim_machine_pop(machine, &a);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop(machine, &b);
    slim_machine_except(machine, error);

    u64_t result = a * b;

    error = ___slim_machine_push(machine, result);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_div(SlimMachine* machine, SlimInstruction instruction) {
    printf("DIV\n");

    u64_t a;
    u64_t b;
    SlimError error;

    error = ___slim_machine_pop(machine, &a);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop(machine, &b);
    slim_machine_except(machine, error);

    u64_t result = a / b;

    error = ___slim_machine_push(machine, result);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_addf(SlimMachine* machine, SlimInstruction instruction) {
    slim_todo();
    return;
}

void slim_routine_subf(SlimMachine* machine, SlimInstruction instruction) {
    slim_todo();
    return;
}

void slim_routine_mulf(SlimMachine* machine, SlimInstruction instruction) {
    slim_todo();
    return;
}

void slim_routine_divf(SlimMachine* machine, SlimInstruction instruction) {
    slim_todo();
    return;
}

void slim_routine_alloc(SlimMachine* machine, SlimInstruction instruction) {
    printf("ALLOC %d\n", instruction.arg1);

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
    printf("FREE %d\n", instruction.arg1);

    SlimError error = ___slim_machine_free(machine, instruction.arg1);
    slim_machine_except(machine, error);

    return;
}

void slim_routine_jmp(SlimMachine* machine, SlimInstruction instruction) {
    printf("JUMP %d\n", instruction.arg2);

    u32_t address = instruction.arg2;
    machine->instruction_pointer = address;
}

void slim_routine_jne(SlimMachine* machine, SlimInstruction instruction) {
    printf("JNE %d\n", instruction.arg1);

    u64_t value;
    SlimError error = ___slim_machine_pop(machine, &value);
    slim_machine_except(machine, error);

    if (value != 0) {
        machine->instruction_pointer = instruction.arg1;
    }
}

void slim_routine_je(SlimMachine* machine, SlimInstruction instruction) {
    printf("JE %d\n", instruction.arg1);

    u64_t value;
    SlimError error = ___slim_machine_pop(machine, &value);
    slim_machine_except(machine, error);

    if (value == 0) {
        machine->instruction_pointer = instruction.arg1;
    }
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
    // Reverse the bits
    arg1 = ((arg1 & 0x000000FF) << 24) | ((arg1 & 0x0000FF00) << 8) | ((arg1 & 0x00FF0000) >> 8) |
           ((arg1 & 0xFF000000) >> 24);

    // Read in the next 4 bytes as the second argument
    u32_t arg2 = 0;
    for (int i = 0; i < 4; i++) {
        arg2 |= machine->bytecode[machine->instruction_pointer + 5 + i] << (i * 8);
    }
    // Reverse the bits
    arg2 = ((arg2 & 0x000000FF) << 24) | ((arg2 & 0x0000FF00) << 8) | ((arg2 & 0x00FF0000) >> 8) |
           ((arg2 & 0xFF000000) >> 24);

    instruction.arg1 = arg1;
    instruction.arg2 = arg2;

    printf("Fetch: 0x%x 0x%x 0x%x\n", instruction.opcode, instruction.arg1, instruction.arg2);

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
    case SL_OPCODE_ADDF: return slim_routine_addf; break;
    case SL_OPCODE_SUBF: return slim_routine_subf; break;
    case SL_OPCODE_MULF: return slim_routine_mulf; break;
    case SL_OPCODE_DIVF: return slim_routine_divf; break;
    case SL_OPCODE_ALLOC: return slim_routine_alloc; break;
    case SL_OPCODE_FREE: return slim_routine_free; break;
    case SL_OPCODE_JMP: return slim_routine_jmp; break;
    case SL_OPCODE_JNE: return slim_routine_jne; break;
    case SL_OPCODE_JE: return slim_routine_je; break;
    default: return NULL; break;
    }
}

void slim_machine_execute(SlimMachine* machine, SlimRoutine routine, SlimInstruction instruction) {
    if (routine) {
        routine(machine, instruction);
    } else {
        printf("Invalid instruction\n");
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
    for (u32_t i = 0; i < SLIM_MACHINE_STACK_SIZE; i++) {
        machine->stack[i] = 0;
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
    machine->stack_pointer = 0;
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
    }
    printf("Machine halted\n");
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
void slim_machine_dump_stack(SlimMachine* machine) {
    printf("Stack:\n");
    for (u32_t i = 0; i < SLIM_MACHINE_STACK_SIZE; i++) {
        printf("%d: %llu\n", i, machine->stack[i]);
    }

    printf("Stack Pointer: %llu\n", (s64_t)machine->stack_pointer);

    printf("\n");
}

void slim_machine_dump_registers(SlimMachine* machine) {
    printf("Registers:\n");
    for (u32_t i = 0; i < SLIM_MACHINE_REGISTERS; i++) {
        printf("%d: %llu\n", i, machine->registers[i]);
    }

    printf("\n");
}

void slim_machine_dump_memory(SlimMachine* machine) {
    printf("Memory:\n");
    for (u32_t i = 0; i < SLIM_MACHINE_MEMORY_SIZE; i++) {
        printf("%d: %llu\n", i, machine->memory[i]);
    }

    printf("\n");
}