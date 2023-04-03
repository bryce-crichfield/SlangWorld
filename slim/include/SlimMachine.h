#pragma once

#include <SlimType.h>

#include <stdio.h>
#include <stdlib.h>


// ---------------------------------------------------------------------------------------------------------------------

/** The public API for the machine module.  The machine module encapsulates the inner workings of a single virtual
 *  machine process.  It is responsible for execution of bytecode and the management of its own memory.
 *  The machine module presents a highly simplified interface to the rest of the system.
 *  The public API must provide for three key operations:
 *  1. Lifetime management - providing for the creation and destruction of a machine
 *  2. Execution management - providing for the execution of a machine and the loading of bytecode
 *  3. Memory management - providing simplified access to the operand stack (primarily for native functions)
 */

typedef struct SlimMachineState SlimMachineState;
typedef struct SlimMachineFlags SlimMachineFlags;

struct SlimMachineFlags {
    u16_t interrupt : 1; // raised by the bytecode when an interrupt or native function is called
    u16_t error : 1;     // raised by the bytecode when an error occurs
    u16_t halt : 1;      // raised by the bytecode when the program is finished
};

// TODO: Find me a better home when native code is implemented
u8_t* slim_bytecode_load(const char* filename, u32_t* size);

SlimMachineState* slim_machine_create();
void slim_machine_reset(SlimMachineState* machine);
void slim_machine_destroy(SlimMachineState* machine);

void slim_machine_step(SlimMachineState* machine);
void slim_machine_load(SlimMachineState* machine, u8_t* data, u32_t size);
void slim_machine_get_flags(SlimMachineState* machine, SlimMachineFlags* flags);

SlimError slim_machine_push(SlimMachineState* machine, u64_t value);
SlimError slim_machine_pop(SlimMachineState* machine, u64_t* value);

// ---------------------------------------------------------------------------------------------------------------------

#define SLIM_MACHINE_OPERAND_STACK_SIZE 8
#define SLIM_MACHINE_CALL_STACK_SIZE 8
#define SLIM_MACHINE_REGISTERS 4
#define SLIM_MACHINE_MEMORY_SIZE 16

typedef struct SlimCallStackFrame SlimCallStackFrame;
typedef struct SlimInstruction SlimInstruction;
typedef struct SlimBytecode SlimBytecode;
typedef enum SlimOpcode SlimOpcode;
typedef struct SlimBlock SlimBlock;
typedef void (*SlimRoutine)(SlimMachineState* machine, SlimInstruction instruction);
// State and Data - Machine, Errors, and Memory ------------------------------------------------------------------------

struct SlimCallStackFrame {
    u32_t instruction_pointer;
    u32_t size;
};

struct SlimMachineState {
    SlimMachineFlags flags;

    // We will use a 32-bit address space which is realistically too large.  In order to access the full 32-bits,
    // we will need to implement a page table.  For now, this is entirely ignored.
    u32_t operand_stack_pointer;
    u32_t call_stack_pointer;
    u32_t instruction_pointer;

    // We will use an unsigned 64-bit value to stand in for all values.  It is up to the user to ensure type safety.
    u64_t operand_stack[SLIM_MACHINE_OPERAND_STACK_SIZE];        // The actual values are stored here
    SlimCallStackFrame call_stack[SLIM_MACHINE_CALL_STACK_SIZE]; // The size of the current call is stored here
    u64_t registers[SLIM_MACHINE_REGISTERS];

    SlimBlock* blocks;
    u64_t memory[SLIM_MACHINE_MEMORY_SIZE];

    u8_t* bytecode;
    u32_t bytecode_size;
};

// Fetch, Decode, Execute
SlimInstruction slim_machine_fetch(SlimMachineState* machine);
SlimRoutine slim_machine_decode(SlimMachineState* machine, SlimInstruction instruction);
void slim_machine_execute(SlimMachineState* machine, SlimRoutine routine, SlimInstruction instruction);

// Internal API - Called by routines to manipulate the machine state

SlimError ___slim_machine_push_operand(SlimMachineState* machine, u64_t value);
SlimError ___slim_machine_pop_operand(SlimMachineState* machine, u64_t* value);
SlimError ___slim_machine_load_register(SlimMachineState* machine, u32_t register);
SlimError ___slim_machine_store_register(SlimMachineState* machine, u32_t register);
SlimError ___slim_machine_read_memory(SlimMachineState* machine, u32_t address, u32_t offset);
SlimError ___slim_machine_write_memory(SlimMachineState* machine, u32_t address, u32_t offset);
SlimError ___slim_machine_alloc_memory(SlimMachineState* machine, u32_t size, u32_t* address);
SlimError ___slim_machine_free_memory(SlimMachineState* machine, u32_t address);
SlimError ___slim_machine_call_function(SlimMachineState* machine, u32_t address);
SlimError ___slim_machine_ret_function(SlimMachineState* machine);

// Logic and Control Flow - Instructions, Routines, and Opcodes --------------------------------------------------------

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

    SL_OPCODE_ITOF      = 0x70,     // Convert the top of the stack from integer to float       ITOF
    SL_OPCODE_FTOI      = 0x71,     // Convert the top of the stack from float to integer       FTOI
    // clang-format on
};

struct SlimInstruction {
    u8_t opcode;
    u32_t arg1;
    u32_t arg2;
};

void slim_routine_nop(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_halt(SlimMachineState* machine, SlimInstruction instruction);

void slim_routine_loadi(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_loadr(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_loadm(SlimMachineState* machine, SlimInstruction instruction);

void slim_routine_drop(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_storer(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_storem(SlimMachineState* machine, SlimInstruction instruction);

void slim_routine_dup(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_swap(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_rot(SlimMachineState* machine, SlimInstruction instruction);

void slim_routine_add(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_sub(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_mul(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_div(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_mod(SlimMachineState* machine, SlimInstruction instruction);

void slim_routine_addf(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_subf(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_mulf(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_divf(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_modf(SlimMachineState* machine, SlimInstruction instruction);

void slim_routine_alloc(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_free(SlimMachineState* machine, SlimInstruction instruction);

void slim_routine_jmp(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_jne(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_je(SlimMachineState* machine, SlimInstruction instruction);

void slim_routine_call(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_ret(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_calln(SlimMachineState* machine, SlimInstruction instruction);

void slim_routine_ftoi(SlimMachineState* machine, SlimInstruction instruction);
void slim_routine_itof(SlimMachineState* machine, SlimInstruction instruction);

// Block and Memory Management -----------------------------------------------------------------------------------------

struct SlimBlock {
    u8_t allocated;
    u32_t start;
    u32_t end;
    SlimBlock* next;
};

SlimBlock* slim_block_create(u32_t start, u32_t end);
void slim_block_destroy(SlimBlock* block);
SlimError slim_block_split(SlimBlock* block, u32_t size);
SlimError slim_block_merge(SlimBlock* block);
