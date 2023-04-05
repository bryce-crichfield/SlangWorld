#pragma once

#include <SlimType.h>

#include <stdio.h>
#include <stdlib.h>

/** --------------------------------------------------------------------------------------------------------------------
 *  The public API for the machine module.  The machine module encapsulates the inner workings of a single virtual
 *  machine process.  It is responsible for execution of bytecode and the management of its own memory.
 *  The machine module presents a highly simplified interface to the rest of the system.
 *  The public API must provide for three key operations:
 *  1. Lifetime management - providing for the creation and destruction of a machine
 *  2. Execution management - providing for the execution of a machine and the loading of bytecode
 *  3. Memory management - providing simplified access to the operand stack (primarily for native functions)
 * ------------------------------------------------------------------------------------------------------------------ */

typedef struct SlimMachineState SlimMachineState;
typedef struct SlimMachineFlags SlimMachineFlags;
typedef struct SlimMachineStackFrame SlimMachineStackFrame;
typedef struct SlimMachineInstruction SlimMachineInstruction;
struct SlimMachineInstruction {
    u8_t opcode;
    u32_t arg1;
    u32_t arg2;
};
typedef enum SlimOpcode SlimOpcode;
typedef enum SlimRuntimeCastArg SlimRuntimeCastArg;
typedef struct SlimMachineBlock SlimMachineBlock;
typedef void (*SlimMachineRoutine)(SlimMachineState* machine, SlimMachineInstruction instruction);

// TODO: Find me a better home when native code is implemented
u8_t* slim_bytecode_load(const char* filename, u32_t* size);

SlimMachineState* slim_machine_create();
void slim_machine_reset(SlimMachineState* machine);
void slim_machine_destroy(SlimMachineState* machine);

void slim_machine_step(SlimMachineState* machine);
void slim_machine_load(SlimMachineState* machine, u8_t* data, u32_t size);

u8_t slim_machine_flag_error_get(SlimMachineState* machine);
u8_t slim_machine_flag_interrupt_get(SlimMachineState* machine);
u8_t slim_machine_flag_halt_get(SlimMachineState* machine);

SlimError slim_machine_push(SlimMachineState* machine, u64_t value);
SlimError slim_machine_pop(SlimMachineState* machine, u64_t* value);

// Logic and Control Flow - Instructions, Routines, and Opcodes --------------------------------------------------------
// This is public because it is shared with the intermediate representation produced by the compiler
enum SlimRuntimeCastArg {
    SLIM_RUNTIME_CAST_ARG_INTEGER = 0,
    SLIM_RUNTIME_CAST_ARG_FLOAT = 1,
    SLIM_RUNTIME_CAST_ARG_STRING = 2,
};

enum SlimOpcode {
    // clang-format off
    SL_OPCODE_NOOP      = 0x00,     // No operation                                             NOOP                                     
    SL_OPCODE_HALT      = 0x01,     // Halt the machine                                         HALT 

    SL_OPCODE_LOADI     = 0x10,     // Load onto stack using Immediate Mode                     LOADI VALUE
    SL_OPCODE_LOADR     = 0x11,     // Load onto stack using Register Mode                      LOADR REG
    SL_OPCODE_LOADM     = 0x12,     // Load onto stack using Memory Mode                        LOADM ADDR FIELD_OFFSET
    SL_OPCODE_DROP      = 0x13,     // Drop the top of the stack                                DROP
    SL_OPCODE_STORER    = 0x14,     // Store the 2nd of the stack in the register from 1st      STORER [0] [1]
    SL_OPCODE_STOREM    = 0x15,     // Store the 2nd of the stack in the address from 1st       STOREM [0] [1] FIELD_OFFSET

    SL_OPCODE_DUP       = 0x20,     // Duplicate the top of the stack                           DUP
    SL_OPCODE_SWAP      = 0x21,     // Swap the top two values on the stack                     SWAP
    SL_OPCODE_ROT       = 0x22,     // Rotate the top three values on the stack                 ROT3

    SL_OPCODE_ADD       = 0x30,     // Add the top two values on the stack as integers          ADD
    SL_OPCODE_SUB       = 0x31,     // Subtract the top two values on the stack as integers     SUBI
    SL_OPCODE_MUL       = 0x32,     // Multiply the top two values on the stack as integers     MULI
    SL_OPCODE_DIV       = 0x33,     // Divide the top two values on the stack as integers       DIVI
    SL_OPCODE_MOD       = 0x34,     // Modulo the top two values on the stack as integers       MODI
    
    SL_OPCODE_ADDF      = 0x35,     // Add the top two values on the stack as floats            ADDF
    SL_OPCODE_SUBF      = 0x36,     // Subtract the top two values on the stack as floats       SUBF
    SL_OPCODE_MULF      = 0x37,     // Multiply the top two values on the stack as floats       MULF
    SL_OPCODE_DIVF      = 0x38,     // Divide the top two values on the stack as floats         DIVF
    SL_OPCODE_MODF      = 0x39,     // Modulo the top two values on the stack as floats         MODF

    SL_OPCODE_ALLOC     = 0x40,     // Allocate memory, return address to top of stack          ALLOC SIZE
    SL_OPCODE_FREE      = 0x41,     // Free memory at address on top of stack                   FREE 

    SL_OPCODE_JMP       = 0x50,     // Jump to specified address                                JMP ADDR
    SL_OPCODE_JNE       = 0x51,     // Jump to specified address if stack top not equal to zero JNE ADDR
    SL_OPCODE_JE        = 0x52,     // Jump to specified address if stack top equal to zero     JE ADDR

    SL_OPCODE_CALL      = 0x60,     // Call a function at the address from the top of stack     CALL 
    SL_OPCODE_RET       = 0x61,     // Return from a function                                   RET
    SL_OPCODE_CALLN     = 0x62,     // Call a native function from the native function table    CALLN NATIVE_FUNCTION_INDEX

    SL_OPCODE_CAST      = 0x70,     // Cast the top of the stack to the specified type          CAST TO FROM (SEE SLIM_RUNTIME_TYPE_*)
    // clang-format on
};

/** --------------------------------------------------------------------------------------------------------------------
 * @brief The internal API for the SLIM machine.  This API is not intended to be used by the rest of the system.  It is
 * intended to be used by the machine module itself.  The internal API is responsible for the execution of bytecode
 * and the management of the machine's memory.
 * ------------------------------------------------------------------------------------------------------------------ */

SlimMachineInstruction ___slim_machine_fetch(SlimMachineState* machine);
SlimMachineRoutine ___slim_machine_decode(SlimMachineState* machine, SlimMachineInstruction instruction);
void ___slim_machine_execute(SlimMachineState* machine, SlimMachineRoutine routine, SlimMachineInstruction instruction);

void ___slim_machine_flag_error_raise(SlimMachineState* machine);
SlimError ___slim_machine_bytecode_jump(SlimMachineState* machine, u32_t address);
SlimError ___slim_machine_operand_push(SlimMachineState* machine, u64_t value);
SlimError ___slim_machine_operand_pop(SlimMachineState* machine, u64_t* value);
SlimError ___slim_machine_register_load(SlimMachineState* machine, u32_t register);
SlimError ___slim_machine_register_store(SlimMachineState* machine, u32_t register);
SlimError ___slim_machine_memory_read(SlimMachineState* machine, u32_t address, u32_t offset);
SlimError ___slim_machine_memory_write(SlimMachineState* machine, u32_t address, u32_t offset);
SlimError ___slim_machine_memory_alloc(SlimMachineState* machine, u32_t size, u32_t* address);
SlimError ___slim_machine_memory_free(SlimMachineState* machine, u32_t address);
SlimError ___slim_machine_function_call(SlimMachineState* machine, u32_t address);
SlimError ___slim_machine_function_ret(SlimMachineState* machine);

/** --------------------------------------------------------------------------------------------------------------------
 * @brief A routine is a function that is called when a specific opcode is encountered.
 * Defined in SlimRoutine.c
 * ------------------------------------------------------------------------------------------------------------------ */

void slim_machine_routine_nop(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_halt(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_loadi(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_loadr(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_loadm(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_drop(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_storer(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_routine_storem(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_dup(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_swap(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_rot(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_add(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_sub(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_mul(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_div(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_mod(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_addf(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_subf(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_mulf(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_divf(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_modf(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_alloc(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_free(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_jmp(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_jne(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_je(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_call(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_ret(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_calln(SlimMachineState* machine, SlimMachineInstruction instruction);
void slim_machine_routine_cast(SlimMachineState* machine, SlimMachineInstruction instruction);

// Block and Memory Management -----------------------------------------------------------------------------------------

SlimMachineBlock* slim_machine_block_create(u32_t start, u32_t end);
void slim_machine_block_destroy(SlimMachineBlock* block);
SlimError slim_machine_block_split(SlimMachineBlock* block, u32_t size);
SlimError slim_machine_block_merge(SlimMachineBlock* block);
