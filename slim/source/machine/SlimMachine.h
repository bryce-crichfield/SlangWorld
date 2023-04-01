#pragma once

#include "../SlimType.h"

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