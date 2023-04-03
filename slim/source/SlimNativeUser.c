#include <SlimNative.h>

#include <stdio.h>
#include <stdlib.h>

// SlimError slim_native_print_num() {
//     // Pop the number off the stack
//     u64_t number;
//     SlimError error = slim_native_pop(&number);
//     if (error != SL_ERROR_NONE) {
//         return error;
//     }

//     // Print the number
//     printf("%llu", number);
//     printf("\n");
//     return SL_ERROR_NONE;
// }

// /** @brief The user has to define this method somewhere in the compilation chain for SLIM to work...
//  * Really, I hate this solution, and in the future the bindings should be defined dynamically in a DLL or SO.
//  * But for now, this is the best solution I can come up with.
// */
void slim_native_user_defintion() { printf("Hello World!\n"); }