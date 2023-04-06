#include <SlimBytecode.h>
#include <SlimLog.h>
#include <SlimMachine.h>
#include <SlimPlatform.h>

#include <stdlib.h>
// ---------------------------------------------------------------------------------------------------------------------
struct SlimPlatform {
    SlimMachineState machine;
    SlimBytecodeTable bytecode_table;
    SlimLogContext log_context;
};
// ---------------------------------------------------------------------------------------------------------------------
SlimPlatform slim_platform_create(int argc, char** argv)
{
    if (argc < 3) {
        printf("Usage: slim <bytecode> <log>\n");
        return NULL;
    }

    SlimPlatform platform = malloc(sizeof(struct SlimPlatform));

    platform->log_context = slim_log_create(argv[2], 1);

    platform->machine = slim_machine_create(&platform->log_context);

    platform->bytecode_table = slim_bytecode_table_create();

    // TODO: Load bytecode tables

    slim_log_using_context(&platform->log_context);
    slim_log_info("[PLATFORM]\tPlatform created\n");

    return platform;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimPlatformReturnCode slim_platform_update(SlimPlatform platform)
{
    slim_log_using_context(&platform->log_context);
    slim_log_flush(platform->log_context);

    SlimPlatformReturnCode return_code = SLIM_PLATFORM_CONTINUE;

    return_code = ___slim_platform_handle_flags(platform);
    if (return_code != SLIM_PLATFORM_CONTINUE) {
        return return_code;
    }

    return_code = ___slim_platform_handle_interrupts(platform);
    if (return_code != SLIM_PLATFORM_CONTINUE) {
        return return_code;
    }

    slim_machine_step(platform->machine, platform->bytecode_table);

    return return_code;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_platform_destroy(SlimPlatform platform)
{
    slim_log_destroy(platform->log_context);
    slim_machine_destroy(platform->machine);
    // TODO: Integrate cleanup of the bytecode and natives
}
// ---------------------------------------------------------------------------------------------------------------------
SlimPlatformReturnCode ___slim_platform_handle_flags(SlimPlatform platform)
{
    // First handle any flags set during the last update and react accordingly
    // For now, the platform is not resposible for clearing/resetting the flags
    slim_log_using_context(&platform->log_context);

    u8_t error_flag = slim_machine_flag_get_error(platform->machine);
    if (error_flag) {
        slim_log_error("[UPDATE]\tRuntime error encountered, terminating platform\n");
        return SLIM_PLATFORM_ERROR;
    }

    u8_t halt_flag = slim_machine_flag_get_halt(platform->machine);
    if (halt_flag) {
        slim_log_info("[UPDATE]\tMachine halted, terminating platform\n");
        return SLIM_PLATFORM_EXIT;
    }

    return SLIM_PLATFORM_CONTINUE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimPlatformReturnCode ___slim_platform_handle_interrupts(SlimPlatform platform)
{
    // Handle interrupt at the level of the platform.  This is because interrupts can cause an error, which would
    // can terminate the platform.  If the interrupt is handled at the machine level, then the machine would have
    // to handle the error and then return to the platform, which would then have to handle the error anyways.
    // Let's make the platform handle it for better control flow.
    slim_log_using_context(&platform->log_context);

    u8_t interrupt_flag = slim_machine_flag_get_interrupt(platform->machine);
    if (interrupt_flag) {
        SlimError error = SL_ERROR_NONE;

        // TODO: Pop native function identifier from stack
        if (error != SL_ERROR_NONE) {
            slim_log_error("[INTERRUPT]\tFailed to pop native function identifier from stack\n");
            return SLIM_PLATFORM_ERROR;
        }

        // TODO: Retrieve native function
        if (error != SL_ERROR_NONE) {
            slim_log_error("[INTERRUPT]\tFailed to retrieve native function\n");
            return SLIM_PLATFORM_ERROR;
        }

        // TODO: Execute native function
        if (error != SL_ERROR_NONE) {
            slim_log_error("[INTERRUPT]\tNative function returned error: %d\n", error);
            return SLIM_PLATFORM_ERROR;
        }
    }

    return SLIM_PLATFORM_CONTINUE;
}
// ---------------------------------------------------------------------------------------------------------------------