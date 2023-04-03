#include <SlimLog.h>
#include <SlimMachine.h>
#include <SlimType.h>

#include <math.h>
// ---------------------------------------------------------------------------------------------------------------------
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
void slim_machine_routine_nop(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tNOP\n");
    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_halt(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tHALT\n");
    ___slim_machine_flag_error_raise(machine);
    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_loadi(SlimMachineState* machine, SlimMachineInstruction instruction) {
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
void slim_machine_routine_loadr(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tLOADR %d\n", instruction.arg1);

    u32_t index = instruction.arg1;

    SlimError error = ___slim_machine_register_load(machine, index);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_loadm(SlimMachineState* machine, SlimMachineInstruction instruction) {
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
void slim_machine_routine_drop(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tDROP\n");

    SlimError error = ___slim_machine_operand_pop(machine, NULL);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_storer(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tSTORER %d\n", instruction.arg1);

    u32_t index = instruction.arg1;
    SlimError error;

    error = ___slim_machine_register_store(machine, index);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_routine_storem(SlimMachineState* machine, SlimMachineInstruction instruction) {
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
void slim_machine_routine_dup(SlimMachineState* machine, SlimMachineInstruction instruction) {
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
void slim_machine_routine_swap(SlimMachineState* machine, SlimMachineInstruction instruction) {
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
void slim_machine_routine_rot(SlimMachineState* machine, SlimMachineInstruction instruction) {
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
void slim_machine_routine_add(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tADD\n");
    ___slim_routine_binary_integer(+);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_sub(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tSUB\n");
    ___slim_routine_binary_integer(-);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_mul(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tMUL\n");
    ___slim_routine_binary_integer(*);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_div(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tDIV\n");
    ___slim_routine_binary_integer(/);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_mod(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tMOD\n");
    ___slim_routine_binary_integer(%);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_addf(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tADDF\n");
    ___slim_routine_binary_float(+);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_subf(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tSUBF\n");
    ___slim_routine_binary_float(-);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_mulf(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tMULF\n");
    ___slim_routine_binary_float(*);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_divf(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tDIVF\n");
    ___slim_routine_binary_float(/);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_modf(SlimMachineState* machine, SlimMachineInstruction instruction) {
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
void slim_machine_routine_alloc(SlimMachineState* machine, SlimMachineInstruction instruction) {
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
void slim_machine_routine_free(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tFREE %d\n", instruction.arg1);

    SlimError error = ___slim_machine_memory_free(machine, instruction.arg1);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_jmp(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tJUMP %d\n", instruction.arg2);

    u32_t address = instruction.arg2;
    SlimError error = ___slim_machine_bytecode_jump(machine, address);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_jne(SlimMachineState* machine, SlimMachineInstruction instruction) {
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
void slim_machine_routine_je(SlimMachineState* machine, SlimMachineInstruction instruction) {
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
void slim_machine_routine_call(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tCALL %x\n", instruction.arg2);

    u32_t address = instruction.arg2;
    SlimError error = ___slim_machine_function_call(machine, address);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_ret(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tRET\n");

    SlimError error = ___slim_machine_function_ret(machine);
    slim_machine_except(machine, error);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_calln(SlimMachineState* machine, SlimMachineInstruction instruction) {
    slim_log_info("[ROUTINE]\tCALLN %x\n", instruction.arg2);

    // We need to signal an interrupt to the platform and push the identifier of the function to call
    // The platform will then call the function and push the result back to the machine

    SlimError error = ___slim_machine_operand_push(machine, instruction.arg2);
    slim_machine_except(machine, error);

    ___slim_machine_flag_error_raise(machine);

    return;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_machine_routine_cast(SlimMachineState* machine, SlimMachineInstruction instruction) {
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