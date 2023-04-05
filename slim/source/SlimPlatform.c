#include <SlimLog.h>
#include <SlimMachine.h>
#include <SlimNative.h>
#include <SlimBytecode.h>
#include <SlimPlatform.h>

#include <stdlib.h>

SlimMachineState* slim_platform_machine_instance; // Global Machine State
// SlimBytecode slim_platform_bytecode_instance; // Global Bytecode State

// ---------------------------------------------------------------------------------------------------------------------]
void slim_platform_init(int argc, char** argv)
{
    if (argc != 3) {
        slim_log_error("Usage: slim <log file> <bytecode file>\n");
        exit(1);
    }

    slim_log_init(argv[1], 1);

    SlimError error = slim_native_init(slim_platform_machine_instance);
    if (error != SL_ERROR_NONE) {
        slim_log_error("Failed to initialize native code\n");
        exit(1);
    }

    // Initialize the machine
    slim_platform_machine_instance = slim_machine_create();
    slim_machine_reset(slim_platform_machine_instance);

    // // Load the bytecode file
    // SlimBytecodeFile bytecode_file;
    // error = slim_bytecode_file_create(&bytecode_file, argv[2]);
    // if (error != SL_ERROR_NONE) {
    //     slim_log_error("Failed to load bytecode file\n");
    //     exit(1);
    // }

    // // Create the bytecode struct
    // error = slim_bytecode_create(&slim_platform_bytecode_instance, &bytecode_file);
    // if (error != SL_ERROR_NONE) {
    //     slim_log_error("Failed to create bytecode struct\n");
    //     exit(1);
    // }

    // // Destroy the bytecode file
    // error = slim_bytecode_file_destroy(&bytecode_file);
    // if (error != SL_ERROR_NONE) {
    //     slim_log_error("Failed to destroy bytecode file\n");
    //     exit(1);
    // }

    // // Load the bytecode into the machine
    // u8_t* code_section;
    // u32_t code_section_size;
    // slim_bytecode_section_code_get(&slim_platform_bytecode_instance, &code_section, &code_section_size);
    // slim_machine_load(slim_platform_machine_instance, code_section, code_section_size);
    // // TODO: Bytecode unfreed, because machine takes ownership of underlying data?
    // TODO: Init native code
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_platform_update()
{
    // Flush the log buffer to the file.  This does cause a constant overhead to the machine and should eventually
    // be made more intelligent to only flush when the buffer is full or when a certain amount of time has passed.
    slim_log_flush();

    // First handle any flags set during the last update and react accordingly
    // For now, the platform is not resposible for clearing/resetting the flags
    u8_t error_flag = slim_machine_flag_error_get(slim_platform_machine_instance);
    if (error_flag) {
        slim_log_error("[UPDATE]\tRuntime error encountered, terminating platform\n");
        slim_platform_exit();
    }

    u8_t halt_flag = slim_machine_flag_halt_get(slim_platform_machine_instance);
    if (halt_flag) {
        slim_log_info("[UPDATE]\tMachine halted, terminating platform\n");
        slim_platform_exit();
    }

    u8_t interrupt_flag = slim_machine_flag_interrupt_get(slim_platform_machine_instance);
    if (interrupt_flag) {
        u64_t native_function_identifier;
        SlimError error;

        error = slim_machine_pop(slim_platform_machine_instance, &native_function_identifier);
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
    }

    // Otherwise, continue running the machine by stepping it
    slim_machine_step(slim_platform_machine_instance);
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_platform_exit()
{
    slim_machine_destroy(slim_platform_machine_instance);
    slim_log_close();
    // slim_bytecode_destroy(&slim_platform_bytecode_instance);
    exit(0);
}
// ---------------------------------------------------------------------------------------------------------------------