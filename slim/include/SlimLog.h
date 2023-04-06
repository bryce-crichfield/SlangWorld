#pragma once

#include <SlimType.h>

#define SLIM_LOG_BUFFER_SIZE 1024

typedef struct SlimLogContext* SlimLogContext;

SlimLogContext slim_log_create(const char* output_path, u8_t writes_stdout);
void slim_log_flush(SlimLogContext log);
void slim_log_destroy(SlimLogContext log);
void slim_log_printf(SlimLogContext log, const char* format, ...);
void slim_log_hexdump(SlimLogContext log, void* data, u32_t size, u32_t stride, u8_t is_sparse);

// Macro Subroutines
#define slim_log_using_context(log) SlimLogContext* CURRENT_USING_LOG_CONTEXT = log;

#define slim_log_info(...)                                                                                             \
    if (*CURRENT_USING_LOG_CONTEXT != 0) {                                                                             \
        slim_log_printf(*CURRENT_USING_LOG_CONTEXT, "[INFO]\t");                                                       \
        slim_log_printf(*CURRENT_USING_LOG_CONTEXT, __VA_ARGS__);                                                      \
        slim_log_printf(*CURRENT_USING_LOG_CONTEXT, "\n");                                                             \
    }

#define slim_log_warn(...)                                                                                             \
    if (*CURRENT_USING_LOG_CONTEXT != 0) {                                                                             \
        slim_log_printf(*CURRENT_USING_LOG_CONTEXT, "[WARN]\t");                                                       \
        slim_log_printf(*CURRENT_USING_LOG_CONTEXT, __VA_ARGS__);                                                      \
        slim_log_printf(*CURRENT_USING_LOG_CONTEXT, "\n");                                                             \
    }

#define slim_log_error(...)                                                                                            \
    if (*CURRENT_USING_LOG_CONTEXT != 0) {                                                                             \
        slim_log_printf(*CURRENT_USING_LOG_CONTEXT, "[ERROR]\t");                                                      \
        slim_log_printf(*CURRENT_USING_LOG_CONTEXT, __VA_ARGS__);                                                      \
        slim_log_printf(*CURRENT_USING_LOG_CONTEXT, "\n");                                                             \
    }
