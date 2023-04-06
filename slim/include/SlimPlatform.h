#pragma once

#include <SlimType.h>

// ---------------------------------------------------------------------------------------------------------------------
// The Slim Platform is the interface between the Slim API and the host platform. It manages the internal components
// of the Slim Runtime such as the virtual machine, logging facilities, native interface, etc.
// ---------------------------------------------------------------------------------------------------------------------
typedef struct SlimPlatform* SlimPlatform;

typedef enum SlimPlatformReturnCode {
    SLIM_PLATFORM_EXIT,
    SLIM_PLATFORM_CONTINUE,
    SLIM_PLATFORM_ERROR
} SlimPlatformReturnCode;

SlimPlatform slim_platform_create(int argc, char** argv);
SlimPlatformReturnCode slim_platform_update(SlimPlatform platform);
void slim_platform_destroy(SlimPlatform platform);
// ---------------------------------------------------------------------------------------------------------------------
SlimPlatformReturnCode ___slim_platform_handle_flags(SlimPlatform platform);
SlimPlatformReturnCode ___slim_platform_handle_interrupts(SlimPlatform platform);
// ---------------------------------------------------------------------------------------------------------------------