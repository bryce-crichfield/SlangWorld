#pragma once

#include <SlimType.h>

// Debugging and Diagnostics -------------------------------------------------------------------------------------------
#define SLIM_LOG_BUFFER_SIZE 1024
// Lifecycle Management
void slim_log_init(const char* output_path, u8_t writes_stdout);
void slim_log_flush();
void slim_log_close();
// Subroutines
void slim_log_printf(const char* format, ...);
void slim_log_hexdump(void* data, u32_t size, u32_t stride, u8_t is_sparse);

// Macro Subroutines
// TODO: Add more sophisticated logging routines for nicer formatting and output
// TODO: Log macros should print newline automatically as the macro represent represent entries in the log
#define slim_info(...)                                                                                                 \
    {                                                                                                                  \
        slim_log_printf("[INFO]\t");                                                                                   \
        slim_log_printf(__VA_ARGS__);                                                                                  \
    }

#define slim_warn(...)                                                                                                 \
    {                                                                                                                  \
        slim_log_printf("[WARN]\t");                                                                                   \
        slim_log_printf(__VA_ARGS__);                                                                                  \
    }

#define slim_log_error(...)                                                                                            \
    {                                                                                                                  \
        slim_log_printf("[ERROR]\t");                                                                                  \
        slim_log_printf(__VA_ARGS__);                                                                                  \
    }