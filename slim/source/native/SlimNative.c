#include "SlimNative.h"

#include <stdio.h>
#include <stdlib.h>


typedef struct SlimNativeFunctionTableEntry {
    u64_t identifier;
    SlimNativeFunction function;
} SlimNativeFunctionTableEntry;

// Hash Table 
typedef struct SlimNativeFunctionTable {
    SlimNativeFunctionTableEntry* entries;
    u64_t size;
    u64_t capacity;
} SlimNativeFunctionTable;

u64_t slim_native_hash(u64_t identifier) {
    return identifier;
}

SlimNativeFunctionTable* slim_native_function_table;

SlimError slim_native_init() {
    slim_native_function_table = malloc(sizeof(SlimNativeFunctionTable));
    slim_native_function_table->entries = malloc(sizeof(SlimNativeFunctionTableEntry) * 1024);
    slim_native_function_table->size = 1024;
    slim_native_function_table->capacity = 1024;
    slim_native_user_defintion();
    return SL_ERROR_NONE;
}

void slim_native_close() {
    free(slim_native_function_table->entries);
    free(slim_native_function_table);
}

SlimError slim_native_get_function(u64_t identifier, SlimNativeFunction* function) {
    u64_t hash = slim_native_hash(identifier);
    SlimNativeFunction found = slim_native_function_table->entries[hash].function;
    if (found == NULL) {
        return SLIM_ERROR;
    }

    *function = found;
    return SL_ERROR_NONE;
}

SlimError slim_native_put_function(u64_t identifier, SlimNativeFunction function) {
    u64_t hash = slim_native_hash(identifier);
    slim_native_function_table->entries[hash].identifier = identifier;
    slim_native_function_table->entries[hash].function = function;
    return SL_ERROR_NONE;
}
