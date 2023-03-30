#pragma once

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;
typedef unsigned long long u64_t;

typedef signed char s8_t;
typedef signed short s16_t;
typedef signed int s32_t;
typedef signed long long s64_t;

typedef float f32_t;
typedef double f64_t;
// ---------------------------------------------------------------------------------------------------------------------

#define SLIM_MACHINE_OPERAND_STACK_SIZE 8
#define SLIM_MACHINE_CALL_STACK_SIZE 8
#define SLIM_MACHINE_REGISTERS 4
#define SLIM_MACHINE_MEMORY_SIZE 16

// Error Handling ------------------------------------------------------------------------------------------------------

typedef enum SlimError SlimError;
enum SlimError {
    SL_ERROR_NONE = 0x0,
    SL_ERROR_STACK_OVERFLOW = 0x1,
    SL_ERROR_STACK_UNDERFLOW = 0x2,
    SL_ERROR_INVALID_REGISTER = 0x3,
    SL_ERROR_BLOCK_SPLIT = 0x4,
    SL_ERROR_BLOCK_MERGE = 0x5,
    SL_ERROR_BLOCK_ALLOC = 0x6,
    SL_ERROR_BLOCK_FREE = 0x7,
};

#define slim_todo()                                                                                                    \
    {                                                                                                                  \
        printf("TODO: %s:%d\n", __FILE__, __LINE__);                                                                   \
        exit(0);                                                                                                       \
    }

#define slim_assert(condition, error)                                                                                  \
    if (!condition) {                                                                                                  \
        printf("Assertion failed: %s\n", #condition);                                                                  \
        return error                                                                                                   \
    }

#define slim_machine_except(machine, error)                                                                            \
    {                                                                                                                  \
        if (error != SL_ERROR_NONE) {                                                                                  \
            machine->flags.error = 1;                                                                                  \
            return;                                                                                                    \
        }                                                                                                              \
    }

// ---------------------------------------------------------------------------------------------------------------------

typedef struct SlimMachine SlimMachine;
typedef struct SlimMachineFlags SlimMachineFlags;
typedef struct SlimCallStackFrame SlimCallStackFrame;
typedef struct SlimInstruction SlimInstruction;
typedef struct SlimBytecode SlimBytecode;
typedef enum SlimOpcode SlimOpcode;
typedef struct SlimBlock SlimBlock;

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

    SL_OPCODE_ITOF      = 0x70,     // Convert the top of the stack from integer to float       ITOF
    SL_OPCODE_FTOI      = 0x71,     // Convert the top of the stack from float to integer       FTOI
    // clang-format on
};

struct SlimBytecode {
    u8_t* data;
    u8_t size;
    u16_t bytesize;
};

struct SlimInstruction {
    u8_t opcode;
    u32_t arg1;
    u32_t arg2;
};

typedef void (*SlimRoutine)(SlimMachine* machine, SlimInstruction instruction);

SlimBytecode* slim_bytecode_load(const char* filename);
void slim_bytecode_destroy(SlimBytecode* bytecode);

void slim_routine_nop(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_halt(SlimMachine* machine, SlimInstruction instruction);

void slim_routine_loadi(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_loadr(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_loadm(SlimMachine* machine, SlimInstruction instruction);

void slim_routine_drop(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_storer(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_storem(SlimMachine* machine, SlimInstruction instruction);

void slim_routine_dup(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_swap(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_rot(SlimMachine* machine, SlimInstruction instruction);

void slim_routine_add(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_sub(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_mul(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_div(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_mod(SlimMachine* machine, SlimInstruction instruction);

void slim_routine_addf(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_subf(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_mulf(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_divf(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_modf(SlimMachine* machine, SlimInstruction instruction);

void slim_routine_alloc(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_free(SlimMachine* machine, SlimInstruction instruction);

void slim_routine_jmp(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_jne(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_je(SlimMachine* machine, SlimInstruction instruction);

void slim_routine_call(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_ret(SlimMachine* machine, SlimInstruction instruction);

void slim_routine_ftoi(SlimMachine* machine, SlimInstruction instruction);
void slim_routine_itof(SlimMachine* machine, SlimInstruction instruction);
// State and Data - Machine, Errors, and Memory ------------------------------------------------------------------------

struct SlimMachineFlags {
    u16_t zero : 1;
    u16_t carry : 1;
    u16_t overflow : 1;
    u16_t negative : 1;
    u16_t interrupt : 1;
    u16_t decimal : 1;
    u16_t error : 1;
    u16_t halt : 1;
};

struct SlimCallStackFrame {
    u32_t instruction_pointer;
    u32_t size;
};

struct SlimMachine {
    SlimMachineFlags flags;

    // We will use a 32-bit address space which is realistically too large.  In order to access the full 32-bits,
    // we will need to implement a page table.  For now, this is entirely ignored.
    u32_t operand_stack_pointer;
    u32_t call_stack_pointer;
    u32_t instruction_pointer;

    // We will use an unsigned 64-bit value to stand in for all values.  It is up to the user to ensure type safety.
    u64_t operand_stack[SLIM_MACHINE_OPERAND_STACK_SIZE];                // The actual values are stored here
    SlimCallStackFrame call_stack[SLIM_MACHINE_CALL_STACK_SIZE];         // The size of the current call is stored here
    u64_t registers[SLIM_MACHINE_REGISTERS];

    SlimBlock* blocks;
    u64_t memory[SLIM_MACHINE_MEMORY_SIZE];

    u8_t* bytecode;
    u32_t bytecode_size;
};

// Fetch, Decode, Execute
SlimInstruction slim_machine_fetch(SlimMachine* machine);
SlimRoutine slim_machine_decode(SlimMachine* machine, SlimInstruction instruction);
void slim_machine_execute(SlimMachine* machine, SlimRoutine routine, SlimInstruction instruction);

// External API
SlimMachine* slim_machine_create();
void slim_machine_destroy(SlimMachine* machine);
void slim_machine_clear(SlimMachine* machine);
void slim_machine_load(SlimMachine* machine, u8_t* data, u32_t size);
void slim_machine_launch(SlimMachine* machine);

// Internal API - Called by routines to manipulate the machine
SlimError ___slim_machine_push(SlimMachine* machine, u64_t value);
SlimError ___slim_machine_pop(SlimMachine* machine, u64_t* value);
SlimError ___slim_machine_load(SlimMachine* machine, u32_t register);
SlimError ___slim_machine_store(SlimMachine* machine, u32_t register);
SlimError ___slim_machine_read(SlimMachine* machine, u32_t address, u32_t offset);
SlimError ___slim_machine_write(SlimMachine* machine, u32_t address, u32_t offset);
SlimError ___slim_machine_alloc(SlimMachine* machine, u32_t size, u32_t* address);
SlimError ___slim_machine_free(SlimMachine* machine, u32_t address);
SlimError ___slim_machine_call(SlimMachine* machine, u32_t address);
SlimError ___slim_machine_ret(SlimMachine* machine);

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

// Debugging and Diagnostics -------------------------------------------------------------------------------------------
#define SLIM_DEBUG_BUFFER_SIZE 1024

typedef struct SlimDebugContext SlimDebugContext;

struct SlimDebugContext {
    const char* output_path;
    char buffer[SLIM_DEBUG_BUFFER_SIZE];
    u16_t buffer_index;    
    u8_t writes_stdout;
};

void slim_debug_init(const char* output_path, u8_t writes_stdout);
void slim_debug_flush();
void slim_debug_close();

void slim_debug_printf(const char* format, ...);
void slim_debug_hexdump(void* data, u32_t size, u32_t stride, u8_t is_sparse);

#define slim_info(...) { \
        slim_debug_printf("[INFO]\t");\
        slim_debug_printf(__VA_ARGS__); \
    }

#define slim_warn(...) { \
        slim_debug_printf("[WARN]\t");\
        slim_debug_printf(__VA_ARGS__);\
    }

#define slim_error(...) {                                                                                                                                                                                                             \
    slim_debug_printf("[ERROR]\t");                                                                                    \
    slim_debug_printf(__VA_ARGS__);                                                                                    \
    }

