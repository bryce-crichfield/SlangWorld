#pragma once

// Common Types --------------------------------------------------------------------------------------------------------
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;
typedef unsigned long long u64_t;

typedef signed char s8_t;
typedef signed short s16_t;
typedef signed int s32_t;
typedef signed long long s64_t;

typedef float f32_t;
typedef double f64_t;
// Error Handling ------------------------------------------------------------------------------------------------------
typedef enum SlimError SlimError;
enum SlimError {
    SL_ERROR_NONE = 0x0,
    SLIM_ERROR = 0x1,
};
y
#define slim_todo()                                                                                                    \
    {                                                                                                                  \
        printf("TODO: %s:%d\n", __FILE__, __LINE__);                                                                   \
        exit(0);                                                                                                       \
    }

#define slim_assert(condition, error)                                                                                  \
    if (!condition) {                                                                                                  \
        printf("Assertion failed: %s\n", #condition);                                                                  \
        return error                                                                                                   \
    }
// ---------------------------------------------------------------------------------------------------------------------