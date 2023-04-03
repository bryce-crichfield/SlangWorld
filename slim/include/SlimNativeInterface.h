#pragma once

#include <SlimType.h>

typedef SlimError (*SlimNativeFunction)();

// // TODO: Implement these
// SlimError slim_native_lookup(const char* identifier, SlimNativeFunction* function) { return SL_ERROR_NONE; }
// SlimError slim_native_register(const char* identifier, SlimNativeFunction function) { return SL_ERROR_NONE; }
// SlimError slim_native_push(u64_t operand) { return SL_ERROR_NONE; }
// SlimError slim_native_pop(u64_t* operand) { return SL_ERROR_NONE; }