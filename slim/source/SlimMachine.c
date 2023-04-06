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

    SlimLogContext* log_context;
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
SlimMachineState slim_machine_create(SlimLogContext* log_context)
{
    SlimMachineState machine = malloc(sizeof(SlimMachineState));
    machine->bytecode = NULL;
    machine->bytecode_size = 0;
    machine->blocks = slim_machine_block_create(0, SLIM_MACHINE_MEMORY_SIZE);
    machine->log_context = log_context;

    slim_machine_reset(machine);
    
    return machine;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_destroy(SlimMachineState machine)
{
    if (machine == NULL) {
        return;
    }

    if (machine->bytecode) {
        free(machine->bytecode);
    }

    slim_machine_block_destroy(machine->blocks);

    free(machine);
    machine = NULL;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_reset(SlimMachineState machine)
{
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
void slim_machine_step(SlimMachineState machine, SlimBytecodeTable bytecode_table)
{
    // Reset any flags
    machine->flags.interrupt = 0;
    machine->flags.error = 0;
    machine->flags.halt = 0;

    SlimMachineInstruction instruction = ___slim_machine_fetch(machine, bytecode_table);
    SlimMachineRoutine routine = ___slim_machine_decode(machine, instruction);
    ___slim_machine_execute(machine, routine, instruction);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_load(SlimMachineState machine, u8_t* data, u32_t size)
{
    machine->bytecode = data;
    machine->bytecode_size = size;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_get_flags(SlimMachineState machine, SlimMachineFlags* flags)
{
    flags->interrupt = machine->flags.interrupt;
    flags->error = machine->flags.error;
    flags->halt = machine->flags.halt;
}
// ---------------------------------------------------------------------------------------------------------------------
u8_t slim_machine_flag_get_error(SlimMachineState machine) { return machine->flags.error; }
// ---------------------------------------------------------------------------------------------------------------------
u8_t slim_machine_flag_get_interrupt(SlimMachineState machine) { return machine->flags.interrupt; }
// ---------------------------------------------------------------------------------------------------------------------
u8_t slim_machine_flag_get_halt(SlimMachineState machine) { return machine->flags.halt; }
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_machine_push(SlimMachineState machine, u64_t value)
{
    return ___slim_machine_operand_push(machine, value);
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_machine_pop(SlimMachineState machine, u64_t* value)
{
    return ___slim_machine_operand_pop(machine, value);
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_machine_pop(SlimMachineState machine, u64_t* value);
// Internal Routines ---------------------------------------------------------------------------------------------------
// TODO: slim_machine_step has now been parameterized over SlimMachineInstruction, so we can remove this
SlimMachineInstruction ___slim_machine_fetch(SlimMachineState machine, SlimBytecodeTable bytecode_table)
{
    slim_log_using_context(machine->log_context);
    slim_todo();
    SlimMachineInstruction instruction;
    // instruction.opcode = machine->bytecode[machine->instruction_pointer];

    // // Read in the next 4 bytes as the first argument
    // u32_t arg1 = 0;
    // for (int i = 0; i < 4; i++) {
    //     arg1 |= machine->bytecode[machine->instruction_pointer + 1 + i] << (i * 8);
    // }

    // // TODO: This is a hack to get around endianness issues, replace with detection and conversion
    // // Reverse the bits
    // arg1 = ((arg1 & 0x000000FF) << 24) | ((arg1 & 0x0000FF00) << 8) | ((arg1 & 0x00FF0000) >> 8) |
    //        ((arg1 & 0xFF000000) >> 24);

    // // Read in the next 4 bytes as the second argument
    // u32_t arg2 = 0;
    // for (int i = 0; i < 4; i++) {
    //     arg2 |= machine->bytecode[machine->instruction_pointer + 5 + i] << (i * 8);
    // }

    // // TODO: This is a hack to get around endianness issues, replace with detection and conversion
    // // Reverse the bits
    // arg2 = ((arg2 & 0x000000FF) << 24) | ((arg2 & 0x0000FF00) << 8) | ((arg2 & 0x00FF0000) >> 8) |
    //        ((arg2 & 0xFF000000) >> 24);

    // instruction.arg1 = arg1;
    // instruction.arg2 = arg2;

    // slim_log_info("[FETCH]\t\t0x%x 0x%x 0x%x\n", instruction.opcode, instruction.arg1, instruction.arg2);

    // machine->instruction_pointer += 9;
    return instruction;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimMachineRoutine ___slim_machine_decode(SlimMachineState machine, SlimMachineInstruction instruction)
{
    switch (instruction.opcode) {
    case SL_OPCODE_NOOP: return slim_machine_routine_nop; break;
    case SL_OPCODE_HALT: return slim_machine_routine_halt; break;
    case SL_OPCODE_LOADI: return slim_machine_routine_loadi; break;
    case SL_OPCODE_LOADR: return slim_machine_routine_loadr; break;
    case SL_OPCODE_LOADM: return slim_machine_routine_loadm; break;
    case SL_OPCODE_DROP: return slim_machine_routine_drop; break;
    case SL_OPCODE_STORER: return slim_machine_routine_storer; break;
    case SL_OPCODE_STOREM: return slim_machine_routine_storem; break;
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
void ___slim_machine_execute(SlimMachineState machine, SlimMachineRoutine routine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    if (routine) {
        routine(machine, instruction);
    } else {
        slim_log_error("[EXECUTE]\tInvalid instruction 0x%x\n", instruction.opcode);
        machine->flags.error = 1;
    }
}
// ---------------------------------------------------------------------------------------------------------------------
void ___slim_machine_flag_error_raise(SlimMachineState machine) { machine->flags.error = 1; }
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_bytecode_jump(SlimMachineState machine, u32_t address)
{
    slim_log_using_context(machine->log_context);

    if (address >= machine->bytecode_size) {
        slim_log_error("[JUMP]\t\tInvalid address 0x%x\n", address);
        return SLIM_ERROR;
    }

    machine->instruction_pointer = address;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_operand_push(SlimMachineState machine, u64_t value)
{
    if (machine->operand_stack_pointer >= SLIM_MACHINE_OPERAND_STACK_SIZE) {
        return SLIM_ERROR;
    }

    machine->operand_stack[machine->operand_stack_pointer++] = value;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError ___slim_machine_operand_pop(SlimMachineState machine, u64_t* value)
{
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
SlimError ___slim_machine_register_load(SlimMachineState machine, u32_t index)
{
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
SlimError ___slim_machine_register_store(SlimMachineState machine, u32_t index)
{
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
SlimError ___slim_machine_memory_read(SlimMachineState machine, u32_t address, u32_t offset)
{
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
SlimError ___slim_machine_memory_write(SlimMachineState machine, u32_t address, u32_t offset)
{
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
SlimError ___slim_machine_memory_alloc(SlimMachineState machine, u32_t size, u32_t* address)
{
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
SlimError ___slim_machine_memory_free(SlimMachineState machine, u32_t address)
{
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
SlimError ___slim_machine_function_call(SlimMachineState machine, u32_t address)
{
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
SlimError ___slim_machine_function_ret(SlimMachineState machine)
{
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
SlimMachineBlock* slim_machine_block_create(u32_t start, u32_t end)
{
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
void slim_machine_block_destroy(SlimMachineBlock* block)
{
    if (block->next != NULL) {
        slim_machine_block_destroy(block->next);
    }
    free(block);
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_machine_block_split(SlimMachineBlock* block, u32_t size)
{
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
SlimError slim_machine_block_merge(SlimMachineBlock* block)
{
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
//    _____ _ _                  __  __            _     _                   _____             _   _
//   / ____| (_)                |  \/  |          | |   (_)                 |  __ \           | | (_)
//  | (___ | |_ _ __ ___        | \  / | __ _  ___| |__  _ _ __   ___       | |__) |___  _   _| |_ _ _ __   ___ ___
//   \___ \| | | '_ ` _ \       | |\/| |/ _` |/ __| '_ \| | '_ \ / _ \      |  _  // _ \| | | | __| | '_ \ / _ / __|
//   ____) | | | | | | | |      | |  | | (_| | (__| | | | | | | |  __/      | | \ | (_) | |_| | |_| | | | |  __\__
//  |_____/|_|_|_| |_| |_|      |_|  |_|\__,_|\___|_| |_|_|_| |_|\___|      |_|  \_\___/ \__,_|\__|_|_| |_|\___|___/
// --------------------------------------------------------------------------------------------------------------------

#include <math.h>

#define slim_machine_except(machine, thrownError)                                                                      \
    {                                                                                                                  \
        if (thrownError != SL_ERROR_NONE) {                                                                            \
            ___slim_machine_flag_error_raise(machine);                                                                 \
            return;                                                                                                    \
        }                                                                                                              \
    }
// ---------------------------------------------------------------------------------------------------------------------
// Used to template integer arithmetic operations
#define ___slim_routine_binary_integer(operation)                                                                      \
    u64_t a;                                                                                                           \
    u64_t b;                                                                                                           \
    SlimError error;                                                                                                   \
    error = ___slim_machine_operand_pop(machine, &b);                                                                  \
    slim_machine_except(machine, error);                                                                               \
    error = ___slim_machine_operand_pop(machine, &a);                                                                  \
    slim_machine_except(machine, error);                                                                               \
    u64_t result = a operation b;                                                                                      \
    error = ___slim_machine_operand_push(machine, result);                                                             \
    slim_machine_except(machine, error);

// ---------------------------------------------------------------------------------------------------------------------
// Used to template floating point arithmetic operations
#define ___slim_routine_binary_float(operation)                                                                        \
    u64_t a;                                                                                                           \
    u64_t b;                                                                                                           \
    SlimError error;                                                                                                   \
    error = ___slim_machine_operand_pop(machine, &b);                                                                  \
    slim_machine_except(machine, error);                                                                               \
    error = ___slim_machine_operand_pop(machine, &a);                                                                  \
    slim_machine_except(machine, error);                                                                               \
    double a_double = *((double*)&a);                                                                                  \
    double b_double = *((double*)&b);                                                                                  \
    double result_double = a_double operation b_double;                                                                \
    u64_t result = *((u64_t*)&result_double);                                                                          \
    error = ___slim_machine_operand_push(machine, result);                                                             \
    slim_machine_except(machine, error);
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_nop(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);
    slim_log_info("[ROUTINE]\tNOP\n");
    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_halt(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);
    slim_log_info("[ROUTINE]\tHALT\n");
    ___slim_machine_flag_error_raise(machine);
    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_loadi(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    SlimError error;

    u64_t value = (u64_t)instruction.arg1 << 32 | instruction.arg2;
    slim_log_info("[ROUTINE]\tLOADI 0x%x\n", value);

    error = ___slim_machine_operand_push(machine, value);

    if (error != SL_ERROR_NONE) {
        printf("ERROR: %d\n", error);
    }

    slim_machine_except(machine, error);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_loadr(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tLOADR %d\n", instruction.arg1);

    u32_t index = instruction.arg1;

    SlimError error = ___slim_machine_register_load(machine, index);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_loadm(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tLOADM %d, %d\n", instruction.arg1, instruction.arg2);

    u64_t address = 0;
    // TODO: Which arg is it?
    u32_t offset = instruction.arg1;
    SlimError error;

    error = ___slim_machine_operand_pop(machine, &address);
    slim_machine_except(machine, error);

    error = ___slim_machine_memory_read(machine, (u32_t)address, offset);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_drop(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tDROP\n");

    SlimError error = ___slim_machine_operand_pop(machine, NULL);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_storer(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tSTORER %d\n", instruction.arg1);

    u32_t index = instruction.arg1;
    SlimError error;

    error = ___slim_machine_register_store(machine, index);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_storem(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tSTOREM %d\n", instruction.arg1);

    u64_t address;
    u64_t offset;
    SlimError error;

    error = ___slim_machine_operand_pop(machine, &address);
    slim_machine_except(machine, error);

    slim_machine_except(machine, error);

    offset = instruction.arg1;
    error = ___slim_machine_memory_write(machine, address, offset);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_dup(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tDUP\n");

    u64_t value;
    SlimError error;

    error = ___slim_machine_operand_pop(machine, &value);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_push(machine, value);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_push(machine, value);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_swap(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tSWAP\n");

    u64_t a;
    u64_t b;
    SlimError error;

    error = ___slim_machine_operand_pop(machine, &a);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_pop(machine, &b);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_push(machine, a);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_push(machine, b);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_rot(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tROT\n");

    u64_t a;
    u64_t b;
    u64_t c;
    SlimError error;

    error = ___slim_machine_operand_pop(machine, &a);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_pop(machine, &b);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_pop(machine, &c);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_push(machine, b);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_push(machine, a);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_push(machine, c);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_add(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tADD\n");
    ___slim_routine_binary_integer(+);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_sub(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tSUB\n");
    ___slim_routine_binary_integer(-);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_mul(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tMUL\n");
    ___slim_routine_binary_integer(*);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_div(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tDIV\n");
    ___slim_routine_binary_integer(/);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_mod(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tMOD\n");
    ___slim_routine_binary_integer(%);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_addf(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tADDF\n");
    ___slim_routine_binary_float(+);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_subf(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tSUBF\n");
    ___slim_routine_binary_float(-);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_mulf(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tMULF\n");
    ___slim_routine_binary_float(*);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_divf(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tDIVF\n");
    ___slim_routine_binary_float(/);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_modf(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tMODF\n");

    u64_t a;
    u64_t b;
    SlimError error;

    error = ___slim_machine_operand_pop(machine, &b);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_pop(machine, &a);
    slim_machine_except(machine, error);

    double a_double = *((double*)&a);
    double b_double = *((double*)&b);
    double result_double = fmod(a_double, b_double);
    u64_t result = *((u64_t*)&result_double);

    error = ___slim_machine_operand_push(machine, result);
    slim_machine_except(machine, error);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_alloc(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tALLOC %d\n", instruction.arg1);

    u32_t size = instruction.arg1;
    SlimError error;

    u32_t address;

    error = ___slim_machine_memory_alloc(machine, size, &address);
    slim_machine_except(machine, error);

    error = ___slim_machine_operand_push(machine, address);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_free(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tFREE %d\n", instruction.arg1);

    SlimError error = ___slim_machine_memory_free(machine, instruction.arg1);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_jmp(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tJUMP %d\n", instruction.arg2);

    u32_t address = instruction.arg2;
    SlimError error = ___slim_machine_bytecode_jump(machine, address);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_jne(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tJNE %d\n", instruction.arg1);

    u64_t value;
    SlimError error = ___slim_machine_operand_pop(machine, &value);
    slim_machine_except(machine, error);

    if (value != 0) {
        error = ___slim_machine_bytecode_jump(machine, instruction.arg1);
        slim_machine_except(machine, error);
    }

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_je(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tJE %d\n", instruction.arg1);

    u64_t value;
    SlimError error = ___slim_machine_operand_pop(machine, &value);
    slim_machine_except(machine, error);

    if (value == 0) {
        error = ___slim_machine_bytecode_jump(machine, instruction.arg1);
        slim_machine_except(machine, error);
    }

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_call(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tCALL %x\n", instruction.arg2);

    u32_t address = instruction.arg2;
    SlimError error = ___slim_machine_function_call(machine, address);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_ret(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tRET\n");

    SlimError error = ___slim_machine_function_ret(machine);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_calln(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tCALLN %x\n", instruction.arg2);

    // We need to signal an interrupt to the platform and push the identifier of the function to call
    // The platform will then call the function and push the result back to the machine

    SlimError error = ___slim_machine_operand_push(machine, instruction.arg2);
    slim_machine_except(machine, error);

    ___slim_machine_flag_error_raise(machine);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_cast(SlimMachineState machine, SlimMachineInstruction instruction)
{
    slim_log_using_context(machine->log_context);

    slim_log_info("[ROUTINE]\tCAST %d\n", instruction.arg1);

    u64_t original_value;
    SlimError error = ___slim_machine_operand_pop(machine, &original_value);
    slim_machine_except(machine, error);

    SlimRuntimeCastArg original_type = (SlimRuntimeCastArg)instruction.arg1;
    SlimRuntimeCastArg new_type = (SlimRuntimeCastArg)instruction.arg2;

    u64_t new_value;

    if (original_type == SLIM_RUNTIME_CAST_ARG_INTEGER && new_type == SLIM_RUNTIME_CAST_ARG_FLOAT) {
        double original_value_double = (double)original_value;
        new_value = *((u64_t*)&original_value_double);
    } else if (original_type == SLIM_RUNTIME_CAST_ARG_FLOAT && new_type == SLIM_RUNTIME_CAST_ARG_INTEGER) {
        double original_value_double = *((double*)&original_value);
        new_value = (u64_t)original_value_double;
    } else {
        slim_log_error("Invalid cast from %d to %d\n", original_type, new_type);
        slim_machine_except(machine, SLIM_ERROR);
    }

    return;
}
// ---------------------------------------------------------------------------------------------------------------------