#include "SlimNative.h"
#include "../machine/SlimMachine.h"

#include <stdio.h>
#include <stdlib.h>

SlimError slim_native_printf(SlimMachineState* state) {
    // Pop the number off the stack
    u64_t number;
    SlimError error = slim_machine_pop(state, &number);
    if (error != SL_ERROR_NONE) {
        return error;
    }

    // Print the number
    printf("%llu", number);
    printf("\n");
    return SL_ERROR_NONE;
}

void slim_native_user_defintion() {
    slim_native_put_function(0, slim_native_printf);
    printf("Hello World!\n");

}