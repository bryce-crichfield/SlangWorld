#include "SlimMachine.h"
#include "../log/SlimLog.h"

#include <stdarg.h>
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
    machine->blocks = slim_block_create(0, SLIM_MACHINE_MEMORY_SIZE);
    return machine;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_destroy(SlimMachineState* machine) {
    if (machine->bytecode) {
        free(machine->bytecode);
    }

    slim_block_destroy(machine->blocks);

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
    slim_block_destroy(machine->blocks);
    machine->blocks = slim_block_create(0, SLIM_MACHINE_MEMORY_SIZE);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_step(SlimMachineState* machine) {
    SlimInstruction instruction = slim_machine_fetch(machine);
    SlimRoutine routine = slim_machine_decode(machine, instruction);
    slim_machine_execute(machine, routine, instruction);
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
u8_t* slim_bytecode_load(const char* filename, u32_t* bytecode_size) {
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
    *(bytecode_size) = size / 9;

    return data;
}
// Internal Routines ---------------------------------------------------------------------------------------------------
SlimInstruction slim_machine_fetch(SlimMachineState* machine) {
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

    slim_info("[FETCH]\t\t0x%x 0x%x 0x%x\n", instruction.opcode, instruction.arg1, instruction.arg2);

    machine->instruction_pointer += 9;

    return instruction;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimRoutine slim_machine_decode(SlimMachineState* machine, SlimInstruction instruction) {
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
    case SL_OPCODE_CALLN: return slim_routine_calln; break;
    case SL_OPCODE_ITOF: return slim_routine_itof; break;
    case SL_OPCODE_FTOI: return slim_routine_ftoi; break;
    default: return NULL; break;
    }
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_execute(SlimMachineState* machine, SlimRoutine routine, SlimInstruction instruction) {
    if (routine) {
        routine(machine, instruction);
    } else {
        slim_error("[EXECUTE]\tInvalid instruction 0x%x\n", instruction.opcode);
        machine->flags.error = 1;
    }
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_push_operand(SlimMachineState* machine, u64_t value) {
    if (machine->operand_stack_pointer >= SLIM_MACHINE_OPERAND_STACK_SIZE) {
        return SLIM_ERROR;
    }

    machine->operand_stack[machine->operand_stack_pointer++] = value;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_pop_operand(SlimMachineState* machine, u64_t* value) {
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
SlimError ___slim_machine_load_register(SlimMachineState* machine, u32_t index) {
    if (index >= SLIM_MACHINE_REGISTERS) {
        return SLIM_ERROR;
    }

    u64_t value = machine->registers[index];
    SlimError error = ___slim_machine_push_operand(machine, value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_store_register(SlimMachineState* machine, u32_t index) {
    if (index >= SLIM_MACHINE_REGISTERS) {
        return SLIM_ERROR;
    }

    u64_t value;
    SlimError error = ___slim_machine_pop_operand(machine, &value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    machine->registers[index] = value;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_read_memory(SlimMachineState* machine, u32_t address, u32_t offset) {
    u64_t value;
    SlimError error;

    u64_t* ptr = (u64_t*)(machine->memory + address + offset);
    value = *ptr;

    error = ___slim_machine_push_operand(machine, value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_write_memory(SlimMachineState* machine, u32_t address, u32_t offset) {
    u64_t value;
    SlimError error;

    error = ___slim_machine_pop_operand(machine, &value);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    u64_t* ptr = (u64_t*)(machine->memory + address + offset);
    *ptr = value;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
// TODO: Validate and thouroughly test me
SlimError ___slim_machine_alloc_memory(SlimMachineState* machine, u32_t size, u32_t* address) {
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

    return SLIM_ERROR;
}
// ---------------------------------------------------------------------------------------------------------------------
// TODO: Validate and thouroughly test me
SlimError ___slim_machine_free_memory(SlimMachineState* machine, u32_t address) {
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

    return SLIM_ERROR;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_call_function(SlimMachineState* machine, u32_t address) {
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
SlimError ___slim_machine_ret_function(SlimMachineState* machine) {
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
        SlimError error = ___slim_machine_pop_operand(machine, &value);
        if (error != SL_ERROR_NONE) {
            return error;
        }
    }

    return SL_ERROR_NONE;
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
// ---------------------------------------------------------------------------------------------------------------------
void slim_block_destroy(SlimBlock* block) {
    if (block->next != NULL) {
        slim_block_destroy(block->next);
    }
    free(block);
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_block_split(SlimBlock* block, u32_t size) {
    if (block->allocated) {
        return SLIM_ERROR;
    }

    if (block->end - block->start < size) {
        return SLIM_ERROR;
    }

    SlimBlock* new_block = slim_block_create(block->start + size, block->end);
    if (new_block == NULL) {
        return SLIM_ERROR;
    }

    block->end = block->start + size;
    new_block->next = block->next;
    block->next = new_block;
    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_block_merge(SlimBlock* block) {
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
    SlimBlock* next = block->next;
    block->next = next->next;
    free(next);

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------