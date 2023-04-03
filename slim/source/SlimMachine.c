#include <SlimLog.h>
#include <SlimMachine.h>

#include <stdarg.h>

#define SLIM_MACHINE_OPERAND_STACK_SIZE 8
#define SLIM_MACHINE_CALL_STACK_SIZE 8
#define SLIM_MACHINE_REGISTERS 4
#define SLIM_MACHINE_MEMORY_SIZE 16
// ---------------------------------------------------------------------------------------------------------------------
struct SlimMachineFlags {
    u16_t interrupt : 1; // raised by the bytecode when an interrupt or native function is called
    u16_t error : 1;     // raised by the bytecode when an error occurs
    u16_t halt : 1;      // raised by the bytecode when the program is finished
};

struct SlimMachineStackFrame {
    u32_t instruction_pointer;
    u32_t size;
};

struct SlimMachineBlock {
    u8_t allocated;
    u32_t start;
    u32_t end;
    SlimMachineBlock* next;
};

struct SlimMachineState {
    SlimMachineFlags flags;

    // We will use a 32-bit address space which is realistically too large.  In order to access the full 32-bits,
    // we will need to implement a page table.  For now, this is entirely ignored.
    u32_t operand_stack_pointer;
    u32_t call_stack_pointer;
    u32_t instruction_pointer;

    // We will use an unsigned 64-bit value to stand in for all values.  It is up to the user to ensure type safety.
    u64_t operand_stack[SLIM_MACHINE_OPERAND_STACK_SIZE];           // The actual values are stored here
    SlimMachineStackFrame call_stack[SLIM_MACHINE_CALL_STACK_SIZE]; // The size of the current call is stored here
    u64_t registers[SLIM_MACHINE_REGISTERS];

    SlimMachineBlock* blocks;
    u64_t memory[SLIM_MACHINE_MEMORY_SIZE];

    u8_t* bytecode;
    u32_t bytecode_size;
};
// ---------------------------------------------------------------------------------------------------------------------
#define slim_machine_except(machine, error)                                                                            \
    {                                                                                                                  \
        if (error != SL_ERROR_NONE) {                                                                                  \
            machine->flags.error = 1;                                                                                  \
            return;                                                                                                    \
        }                                                                                                              \
    }
// External API --------------------------------------------------------------------------------------------------------
SlimMachineState* slim_machine_create() {
    SlimMachineState* machine = malloc(sizeof(SlimMachineState));
    machine->bytecode = NULL;
    machine->bytecode_size = 0;
    machine->blocks = slim_machine_block_create(0, SLIM_MACHINE_MEMORY_SIZE);
    return machine;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_destroy(SlimMachineState* machine) {
    if (machine->bytecode) {
        free(machine->bytecode);
    }

    slim_machine_block_destroy(machine->blocks);

    free(machine);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_reset(SlimMachineState* machine) {
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
    machine->flags.interrupt = 0;
    machine->flags.error = 0;
    machine->flags.halt = 0;

    // Reset Pointers
    machine->operand_stack_pointer = 0;
    machine->call_stack_pointer = 0;
    machine->instruction_pointer = 0;

    // Reset Blocks
    slim_machine_block_destroy(machine->blocks);
    machine->blocks = slim_machine_block_create(0, SLIM_MACHINE_MEMORY_SIZE);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_step(SlimMachineState* machine) {
    // Reset any flags
    machine->flags.interrupt = 0;
    machine->flags.error = 0;
    machine->flags.halt = 0;

    SlimMachineInstruction instruction = ___slim_machine_fetch(machine);
    SlimMachineRoutine routine = ___slim_machine_decode(machine, instruction);
    ___slim_machine_execute(machine, routine, instruction);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_load(SlimMachineState* machine, u8_t* data, u32_t size) {
    machine->bytecode = data;
    machine->bytecode_size = size;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_get_flags(SlimMachineState* machine, SlimMachineFlags* flags) {
    flags->interrupt = machine->flags.interrupt;
    flags->error = machine->flags.error;
    flags->halt = machine->flags.halt;
}
// ---------------------------------------------------------------------------------------------------------------------
u8_t slim_machine_flag_error_get(SlimMachineState* machine) { return machine->flags.error; }
// ---------------------------------------------------------------------------------------------------------------------
u8_t slim_machine_flag_interrupt_get(SlimMachineState* machine) { return machine->flags.interrupt; }
// ---------------------------------------------------------------------------------------------------------------------
u8_t slim_machine_flag_halt_get(SlimMachineState* machine) { return machine->flags.halt; }
// ---------------------------------------------------------------------------------------------------------------------
u8_t* slim_bytecode_load(const char* filename, u32_t* bytecode_size) {
    // Open the file

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        slim_log_error("Failed to open file\n");
        return NULL;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    u32_t size = ftell(file);
    if (size % 9 != 0) {
        slim_log_error("Invalid bytecode file\n");
        return NULL;
    }

    // Allocate memory for the bytecode
    u8_t* data = malloc(size);
    if (data == NULL) {
        slim_log_error("Failed to allocate memory for bytecode\n");
        return NULL;
    }

    // Read the file into memory
    fseek(file, 0, SEEK_SET);
    fread(data, size, 1, file);
    fclose(file);

    // Create the bytecode struct
    *(bytecode_size) = size / 9;

    return data;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_machine_push(SlimMachineState* machine, u64_t value) {
    return ___slim_machine_operand_push(machine, value);
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_machine_pop(SlimMachineState* machine, u64_t* value) {
    return ___slim_machine_operand_pop(machine, value);
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_machine_pop(SlimMachineState* machine, u64_t* value);
// Internal Routines ---------------------------------------------------------------------------------------------------
SlimMachineInstruction ___slim_machine_fetch(SlimMachineState* machine) {
    SlimMachineInstruction instruction;
    instruction.opcode = machine->bytecode[machine->instruction_pointer];

    // Read in the next 4 bytes as the first argument
    u32_t arg1 = 0;
    for (int i = 0; i < 4; i++) {
        arg1 |= machine->bytecode[machine->instruction_pointer + 1 + i] << (i * 8);
    }

    // TODO: This is a hack to get around endianness issues, replace with detection and conversion
    // Reverse the bits
    arg1 = ((arg1 & 0x000000FF) << 24) | ((arg1 & 0x0000FF00) << 8) | ((arg1 & 0x00FF0000) >> 8) |
           ((arg1 & 0xFF000000) >> 24);

    // Read in the next 4 bytes as the second argument
    u32_t arg2 = 0;
    for (int i = 0; i < 4; i++) {
        arg2 |= machine->bytecode[machine->instruction_pointer + 5 + i] << (i * 8);
    }

    // TODO: This is a hack to get around endianness issues, replace with detection and conversion
    // Reverse the bits
    arg2 = ((arg2 & 0x000000FF) << 24) | ((arg2 & 0x0000FF00) << 8) | ((arg2 & 0x00FF0000) >> 8) |
           ((arg2 & 0xFF000000) >> 24);

    instruction.arg1 = arg1;
    instruction.arg2 = arg2;

    slim_log_info("[FETCH]\t\t0x%x 0x%x 0x%x\n", instruction.opcode, instruction.arg1, instruction.arg2);

    machine->instruction_pointer += 9;

    return instruction;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimMachineRoutine ___slim_machine_decode(SlimMachineState* machine, SlimMachineInstruction instruction) {
    switch (instruction.opcode) {
    case SL_OPCODE_NOOP: return slim_machine_routine_nop; break;
    case SL_OPCODE_HALT: return slim_machine_routine_halt; break;
    case SL_OPCODE_LOADI: return slim_machine_routine_loadi; break;
    case SL_OPCODE_LOADR: return slim_machine_routine_loadr; break;
    case SL_OPCODE_LOADM: return slim_machine_routine_loadm; break;
    case SL_OPCODE_DROP: return slim_machine_routine_drop; break;
    case SL_OPCODE_STORER: return slim_machine_routine_storer; break;
    case SL_OPCODE_STOREM: return slim_routine_storem; break;
    case SL_OPCODE_DUP: return slim_machine_routine_dup; break;
    case SL_OPCODE_SWAP: return slim_machine_routine_swap; break;
    case SL_OPCODE_ROT: return slim_machine_routine_rot; break;
    case SL_OPCODE_ADD: return slim_machine_routine_add; break;
    case SL_OPCODE_SUB: return slim_machine_routine_sub; break;
    case SL_OPCODE_MUL: return slim_machine_routine_mul; break;
    case SL_OPCODE_DIV: return slim_machine_routine_div; break;
    case SL_OPCODE_MOD: return slim_machine_routine_mod; break;
    case SL_OPCODE_ADDF: return slim_machine_routine_addf; break;
    case SL_OPCODE_SUBF: return slim_machine_routine_subf; break;
    case SL_OPCODE_MULF: return slim_machine_routine_mulf; break;
    case SL_OPCODE_DIVF: return slim_machine_routine_divf; break;
    case SL_OPCODE_MODF: return slim_machine_routine_modf; break;
    case SL_OPCODE_ALLOC: return slim_machine_routine_alloc; break;
    case SL_OPCODE_FREE: return slim_machine_routine_free; break;
    case SL_OPCODE_JMP: return slim_machine_routine_jmp; break;
    case SL_OPCODE_JNE: return slim_machine_routine_jne; break;
    case SL_OPCODE_JE: return slim_machine_routine_je; break;
    case SL_OPCODE_CALL: return slim_machine_routine_call; break;
    case SL_OPCODE_RET: return slim_machine_routine_ret; break;
    case SL_OPCODE_CALLN: return slim_machine_routine_calln; break;
    case SL_OPCODE_CAST: return slim_machine_routine_cast; break;
    default: return NULL; break;
    }
}
// ---------------------------------------------------------------------------------------------------------------------
void ___slim_machine_execute(
    SlimMachineState* machine, SlimMachineRoutine routine, SlimMachineInstruction instruction) {
    if (routine) {
        routine(machine, instruction);
    } else {
        slim_log_error("[EXECUTE]\tInvalid instruction 0x%x\n", instruction.opcode);
        machine->flags.error = 1;
    }
}
// ---------------------------------------------------------------------------------------------------------------------
void ___slim_machine_flag_error_raise(SlimMachineState* machine) { machine->flags.error = 1; }
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_bytecode_jump(SlimMachineState* machine, u32_t address) {
    if (address >= machine->bytecode_size) {
        slim_log_error("[JUMP]\t\tInvalid address 0x%x\n", address);
        return SLIM_ERROR;
    }

    machine->instruction_pointer = address;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_operand_push(SlimMachineState* machine, u64_t value) {
    if (machine->operand_stack_pointer >= SLIM_MACHINE_OPERAND_STACK_SIZE) {
        return SLIM_ERROR;
    }

    machine->operand_stack[machine->operand_stack_pointer++] = value;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_operand_pop(SlimMachineState* machine, u64_t* value) {
    if (machine->operand_stack_pointer == 0) {
        return SLIM_ERROR;
    }

    u64_t top = machine->operand_stack[machine->operand_stack_pointer - 1];
    machine->operand_stack[machine->operand_stack_pointer - 1] = 0;
    machine->operand_stack_pointer--;
    *value = top;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_register_load(SlimMachineState* machine, u32_t index) {
    if (index >= SLIM_MACHINE_REGISTERS) {
        return SLIM_ERROR;
    }

    u64_t value = machine->registers[index];
    SlimError error = ___slim_machine_operand_push(machine, value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_register_store(SlimMachineState* machine, u32_t index) {
    if (index >= SLIM_MACHINE_REGISTERS) {
        return SLIM_ERROR;
    }

    u64_t value;
    SlimError error = ___slim_machine_operand_pop(machine, &value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    machine->registers[index] = value;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_memory_read(SlimMachineState* machine, u32_t address, u32_t offset) {
    u64_t value;
    SlimError error;

    u64_t* ptr = (u64_t*)(machine->memory + address + offset);
    value = *ptr;

    error = ___slim_machine_operand_push(machine, value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_memory_write(SlimMachineState* machine, u32_t address, u32_t offset) {
    u64_t value;
    SlimError error;

    error = ___slim_machine_operand_pop(machine, &value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    u64_t* ptr = (u64_t*)(machine->memory + address + offset);
    *ptr = value;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
// TODO: Validate and thouroughly test me
SlimError ___slim_machine_memory_alloc(SlimMachineState* machine, u32_t size, u32_t* address) {
    SlimMachineBlock* block = machine->blocks;
    while (block != NULL) {
        if (block->allocated == 0 && block->end - block->start >= size) {
            SlimError error = slim_machine_block_split(block, size);
            if (error != SL_ERROR_NONE) {
                return error;
            }

            block->allocated = 1;
            *address = block->start;
            return SL_ERROR_NONE;
        }

        block = block->next;
    }

    return SLIM_ERROR;
}
// ---------------------------------------------------------------------------------------------------------------------
// TODO: Validate and thouroughly test me
SlimError ___slim_machine_memory_free(SlimMachineState* machine, u32_t address) {
    SlimMachineBlock* block = machine->blocks;
    while (block != NULL) {
        if (block->start == address) {
            block->allocated = 0;
            SlimError error = slim_machine_block_merge(block);
            if (error != SL_ERROR_NONE) {
                return error;
            }

            return SL_ERROR_NONE;
        }

        block = block->next;
    }

    return SLIM_ERROR;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_function_call(SlimMachineState* machine, u32_t address) {
    machine->call_stack_pointer++;
    if (machine->call_stack_pointer >= SLIM_MACHINE_CALL_STACK_SIZE) {
        return SLIM_ERROR;
    }

    machine->call_stack[machine->call_stack_pointer].instruction_pointer = machine->instruction_pointer;
    machine->call_stack[machine->call_stack_pointer].size = 0;
    machine->instruction_pointer = address;
    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_function_ret(SlimMachineState* machine) {
    u32_t ret_address = machine->call_stack[machine->call_stack_pointer].instruction_pointer;
    u16_t count = machine->call_stack[machine->call_stack_pointer].size;
    machine->call_stack[machine->call_stack_pointer].instruction_pointer = 0;
    machine->call_stack[machine->call_stack_pointer].size = 0;

    machine->call_stack_pointer--;
    if (machine->call_stack_pointer < 0) {
        return SLIM_ERROR;
    }

    machine->instruction_pointer = ret_address;

    for (u16_t i = 0; i < count; i++) {
        u64_t value;
        SlimError error = ___slim_machine_operand_pop(machine, &value);
        if (error != SL_ERROR_NONE) {
            return error;
        }
    }

    return SL_ERROR_NONE;
}
// Block Management ----------------------------------------------------------------------------------------------------
SlimMachineBlock* slim_machine_block_create(u32_t start, u32_t end) {
    SlimMachineBlock* block = malloc(sizeof(SlimMachineBlock));
    if (block == NULL) {
        return NULL;
    }
    block->allocated = 0;
    block->start = start;
    block->end = end;
    block->next = NULL;
    return block;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_block_destroy(SlimMachineBlock* block) {
    if (block->next != NULL) {
        slim_machine_block_destroy(block->next);
    }
    free(block);
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_machine_block_split(SlimMachineBlock* block, u32_t size) {
    if (block->allocated) {
        return SLIM_ERROR;
    }

    if (block->end - block->start < size) {
        return SLIM_ERROR;
    }

    SlimMachineBlock* new_block = slim_machine_block_create(block->start + size, block->end);
    if (new_block == NULL) {
        return SLIM_ERROR;
    }

    block->end = block->start + size;
    new_block->next = block->next;
    block->next = new_block;
    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_machine_block_merge(SlimMachineBlock* block) {
    if (block->allocated) {
        return SLIM_ERROR;
    }

    if (block->next == NULL) {
        return SLIM_ERROR;
    }

    if (block->next->allocated) {
        return SLIM_ERROR;
    }

    block->end = block->next->end;
    SlimMachineBlock* next = block->next;
    block->next = next->next;
    free(next);

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------