#include <SlimLog.h>
#include <SlimMachine.h>
#include <SlimNative.h>
#include <SlimPlatform.h>

#include <stdlib.h>

SlimMachineState* slim_platform_machine; // Global Machine State

// ---------------------------------------------------------------------------------------------------------------------]
void slim_platform_init(int argc, char** argv) {
    if (argc != 3) {
        slim_log_error("Usage: slim <log file> <bytecode file>\n");
        exit(1);
    }

    slim_log_init(argv[1], 1);

    SlimError error = slim_native_init(slim_platform_machine);
    if (error != SL_ERROR_NONE) {
        slim_log_error("Failed to initialize native code\n");
        exit(1);
    }

    // Initialize the machine
    slim_platform_machine = slim_machine_create();
    slim_machine_reset(slim_platform_machine);

    // Load the bytecode file
    u32_t code_size = 0;
    u8_t* code = slim_bytecode_load(argv[2], &code_size);
    if (code == NULL || code_size == 0) {
        slim_log_error("Failed to load bytecode file: %s\n", argv[2]);
        exit(1);
    }

    // Load the bytecode into the machine
    slim_machine_load(slim_platform_machine, code, code_size);
    // TODO: Bytecode unfreed, because machine takes ownership of underlying data?
    // TODO: Init native code
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_platform_update() {
    // Flush the log buffer to the file.  This does cause a constant overhead to the machine and should eventually
    // be made more intelligent to only flush when the buffer is full or when a certain amount of time has passed.
    slim_log_flush();

    SlimMachineFlags flags;
    slim_machine_get_flags(slim_platform_machine, &flags);

    // First handle any flags set during the last update and react accordingly
    if (flags.error) {
        slim_log_error("[UPDATE]\tRuntime error encountered, terminating platform\n");
        slim_platform_exit();
    }

    if (flags.halt) {
        slim_info("[UPDATE]\tMachine halted, terminating platform\n");
        slim_platform_exit();
    }

    if (flags.interrupt) {
        u64_t native_function_identifier;
        SlimError error;

        error = slim_machine_pop(slim_platform_machine, &native_function_identifier);
        if (error != SL_ERROR_NONE) {
            slim_log_error("[INTERRUPT]\tFailed to pop native function identifier from stack\n");
            slim_platform_exit();
        }

        // TODO: Add native interrupt back in
        // SlimNativeFunction native_function;
        // error = slim_native_get_function(native_function_identifier, &native_function);
        if (error != SL_ERROR_NONE) {
            slim_log_error(
                "[INTERRUPT]\tFailed to get native function for identifier: %llu\n", native_function_identifier);
            slim_platform_exit();
        }

        // error = native_function(slim_platform_machine);
        if (error != SL_ERROR_NONE) {
            slim_log_error("[INTERRUPT]\tNative function returned error: %d\n", error);
            slim_platform_exit();
        }

        flags.interrupt = 0;
    }

    // Otherwise, continue running the machine by stepping it
    slim_machine_step(slim_platform_machine);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_platform_exit() {
    slim_machine_destroy(slim_platform_machine);
    slim_log_close();
    exit(0);
}
// ---------------------------------------------------------------------------------------------------------------------