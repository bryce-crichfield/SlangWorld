#pragma once

#include <SlimType.h>

// ---------------------------------------------------------------------------------------------------------------------
// The Slim Platform is the interface between the Slim API and the host platform. It manages the internal components
// of the Slim Runtime such as the virtual machine, logging facilities, native interface, etc.
// ---------------------------------------------------------------------------------------------------------------------
void slim_platform_init(int argc, char** argv);
void slim_platform_update();
void slim_platform_exit();
// ---------------------------------------------------------------------------------------------------------------------