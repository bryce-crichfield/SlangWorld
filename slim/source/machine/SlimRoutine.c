#include "../SlimType.h"
#include "../log/SlimLog.h"
#include "SlimMachine.h"

#include <math.h>
// ---------------------------------------------------------------------------------------------------------------------
#define slim_machine_except(machine, error)                                                                            \
    {                                                                                                                  \
        if (error != SL_ERROR_NONE) {                                                                                  \
            machine->flags.error = 1;                                                                                  \
            return;                                                                                                    \
        }                                                                                                              \
    }
// ---------------------------------------------------------------------------------------------------------------------
// Used to template integer arithmetic operations
#define ___slim_routine_binary_integer(operation)                                                                      \
    u64_t a;                                                                                                           \
    u64_t b;                                                                                                           \
    SlimError error;                                                                                                   \
    error = ___slim_machine_pop_operand(machine, &b);                                                                  \
    slim_machine_except(machine, error);                                                                               \
    error = ___slim_machine_pop_operand(machine, &a);                                                                  \
    slim_machine_except(machine, error);                                                                               \
    u64_t result = a operation b;                                                                                      \
    error = ___slim_machine_push_operand(machine, result);                                                             \
    slim_machine_except(machine, error);

// ---------------------------------------------------------------------------------------------------------------------
// Used to template floating point arithmetic operations
#define ___slim_routine_binary_float(operation)                                                                        \
    u64_t a;                                                                                                           \
    u64_t b;                                                                                                           \
    SlimError error;                                                                                                   \
    error = ___slim_machine_pop_operand(machine, &b);                                                                  \
    slim_machine_except(machine, error);                                                                               \
    error = ___slim_machine_pop_operand(machine, &a);                                                                  \
    slim_machine_except(machine, error);                                                                               \
    double a_double = *((double*)&a);                                                                                  \
    double b_double = *((double*)&b);                                                                                  \
    double result_double = a_double operation b_double;                                                                \
    u64_t result = *((u64_t*)&result_double);                                                                          \
    error = ___slim_machine_push_operand(machine, result);                                                             \
    slim_machine_except(machine, error);
// ---------------------------------------------------------------------------------------------------------------------
// TODO: Validate and utilize me
#define ___slim_routine_cast(to_type, from_type)                                                                       \
    u64_t a;                                                                                                           \
    SlimError error;                                                                                                   \
    error = ___slim_machine_pop_operand(machine, &a);                                                                  \
    slim_machine_except(machine, error);                                                                               \
    to_type a_cast = (to_type)a;                                                                                       \
    from_type a_cast_cast = (from_type)a_cast;                                                                         \
    u64_t result = (u64_t)a_cast_cast;                                                                                 \
    error = ___slim_machine_push_operand(machine, result);                                                             \
    slim_machine_except(machine, error);
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_nop(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tNOP\n");
    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_halt(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tHALT\n");
    machine->flags.halt = 1;
    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_loadi(SlimMachineState* machine, SlimInstruction instruction) {
    SlimError error;

    u64_t value = (u64_t)instruction.arg1 << 32 | instruction.arg2;
    slim_info("[ROUTINE]\tLOADI 0x%x\n", value);

    error = ___slim_machine_push_operand(machine, value);

    if (error != SL_ERROR_NONE) {
        printf("ERROR: %d\n", error);
    }

    slim_machine_except(machine, error);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_loadr(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tLOADR %d\n", instruction.arg1);

    u32_t index = instruction.arg1;

    SlimError error = ___slim_machine_load_register(machine, index);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_loadm(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tLOADM %d, %d\n", instruction.arg1, instruction.arg2);

    u64_t address = 0;
    // TODO: Which arg is it?
    u32_t offset = instruction.arg1;
    SlimError error;

    error = ___slim_machine_pop_operand(machine, &address);
    slim_machine_except(machine, error);

    error = ___slim_machine_read_memory(machine, (u32_t)address, offset);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_drop(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tDROP\n");

    SlimError error = ___slim_machine_pop_operand(machine, NULL);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_storer(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tSTORER %d\n", instruction.arg1);

    u32_t index = instruction.arg1;
    SlimError error;

    error = ___slim_machine_store_register(machine, index);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_storem(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tSTOREM %d\n", instruction.arg1);

    u64_t address;
    u64_t offset;
    SlimError error;

    error = ___slim_machine_pop_operand(machine, &address);
    slim_machine_except(machine, error);

    slim_machine_except(machine, error);

    offset = instruction.arg1;
    error = ___slim_machine_write_memory(machine, address, offset);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_dup(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tDUP\n");

    u64_t value;
    SlimError error;

    error = ___slim_machine_pop_operand(machine, &value);
    slim_machine_except(machine, error);

    error = ___slim_machine_push_operand(machine, value);
    slim_machine_except(machine, error);

    error = ___slim_machine_push_operand(machine, value);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_swap(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tSWAP\n");

    u64_t a;
    u64_t b;
    SlimError error;

    error = ___slim_machine_pop_operand(machine, &a);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop_operand(machine, &b);
    slim_machine_except(machine, error);

    error = ___slim_machine_push_operand(machine, a);
    slim_machine_except(machine, error);

    error = ___slim_machine_push_operand(machine, b);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_rot(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tROT\n");

    u64_t a;
    u64_t b;
    u64_t c;
    SlimError error;

    error = ___slim_machine_pop_operand(machine, &a);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop_operand(machine, &b);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop_operand(machine, &c);
    slim_machine_except(machine, error);

    error = ___slim_machine_push_operand(machine, b);
    slim_machine_except(machine, error);

    error = ___slim_machine_push_operand(machine, a);
    slim_machine_except(machine, error);

    error = ___slim_machine_push_operand(machine, c);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_add(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tADD\n");
    ___slim_routine_binary_integer(+);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_sub(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tSUB\n");
    ___slim_routine_binary_integer(-);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_mul(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tMUL\n");
    ___slim_routine_binary_integer(*);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_div(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tDIV\n");
    ___slim_routine_binary_integer(/);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_mod(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tMOD\n");
    ___slim_routine_binary_integer(%);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_addf(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tADDF\n");
    ___slim_routine_binary_float(+);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_subf(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tSUBF\n");
    ___slim_routine_binary_float(-);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_mulf(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tMULF\n");
    ___slim_routine_binary_float(*);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_divf(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tDIVF\n");
    ___slim_routine_binary_float(/);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_modf(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tMODF\n");

    u64_t a;
    u64_t b;
    SlimError error;

    error = ___slim_machine_pop_operand(machine, &b);
    slim_machine_except(machine, error);

    error = ___slim_machine_pop_operand(machine, &a);
    slim_machine_except(machine, error);

    double a_double = *((double*)&a);
    double b_double = *((double*)&b);
    double result_double = fmod(a_double, b_double);
    u64_t result = *((u64_t*)&result_double);

    error = ___slim_machine_push_operand(machine, result);
    slim_machine_except(machine, error);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_alloc(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tALLOC %d\n", instruction.arg1);

    u32_t size = instruction.arg1;
    SlimError error;

    u32_t address;

    error = ___slim_machine_alloc_memory(machine, size, &address);
    slim_machine_except(machine, error);

    error = ___slim_machine_push_operand(machine, address);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_free(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tFREE %d\n", instruction.arg1);

    SlimError error = ___slim_machine_free_memory(machine, instruction.arg1);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_jmp(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tJUMP %d\n", instruction.arg2);

    u32_t address = instruction.arg2;
    machine->instruction_pointer = address;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_jne(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tJNE %d\n", instruction.arg1);

    u64_t value;
    SlimError error = ___slim_machine_pop_operand(machine, &value);
    slim_machine_except(machine, error);

    if (value != 0) {
        machine->instruction_pointer = instruction.arg1;
    }
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_je(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tJE %d\n", instruction.arg1);

    u64_t value;
    SlimError error = ___slim_machine_pop_operand(machine, &value);
    slim_machine_except(machine, error);

    if (value == 0) {
        machine->instruction_pointer = instruction.arg1;
    }
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_call(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tCALL %x\n", instruction.arg2);

    u32_t address = instruction.arg2;
    SlimError error = ___slim_machine_call_function(machine, address);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_ret(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tRET\n");

    SlimError error = ___slim_machine_ret_function(machine);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_calln(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tCALLN %x\n", instruction.arg2);

    // We need to signal an interrupt to the platform and push the identifier of the function to call
    // The platform will then call the function and push the result back to the machine

    SlimError error = ___slim_machine_push_operand(machine, instruction.arg2);
    slim_machine_except(machine, error);

    machine->flags.interrupt = 1;
    
    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_ftoi(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tFTOI\n");

    u64_t value;
    SlimError error;

    error = ___slim_machine_pop_operand(machine, &value);
    slim_machine_except(machine, error);

    double value_double = *((double*)&value);
    u64_t result = (u64_t)value_double;

    error = ___slim_machine_push_operand(machine, result);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_itof(SlimMachineState* machine, SlimInstruction instruction) {
    slim_info("[ROUTINE]\tITOF\n");

    u64_t value;
    SlimError error;

    error = ___slim_machine_pop_operand(machine, &value);
    slim_machine_except(machine, error);

    double result_double = (double)value;
    u64_t result = *((u64_t*)&result_double);

    error = ___slim_machine_push_operand(machine, result);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------