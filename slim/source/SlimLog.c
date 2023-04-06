#include <SlimLog.h>

#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------------------------------------------------
struct SlimLogContext {
    const char* output_path;
    char buffer[SLIM_LOG_BUFFER_SIZE];
    u16_t buffer_index;
    u8_t writes_stdout;
};
// ---------------------------------------------------------------------------------------------------------------------
SlimLogContext slim_log_create(const char* output_path, u8_t writes_stdout)
{
    SlimLogContext slim_log_context = malloc(sizeof(SlimLogContext));
    slim_log_context = malloc(sizeof(SlimLogContext));
    slim_log_context->output_path = output_path;
    slim_log_context->buffer_index = 0;
    slim_log_context->writes_stdout = writes_stdout;

    return slim_log_context;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_log_flush(SlimLogContext log)
{
    FILE* file = fopen(log->output_path, "a");
    fwrite(log->buffer, 1, log->buffer_index, file);
    fclose(file);
    log->buffer_index = 0;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_log_destroy(SlimLogContext log)
{
    if (log == NULL) return;
    slim_log_flush(log);
    free(log);
    log = NULL;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_log_printf(SlimLogContext log, const char* format, ...)
{
    if (log == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    log->buffer_index += vsprintf(log->buffer + log->buffer_index, format, args);
    va_end(args);

    if (log->writes_stdout) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

    if (log->buffer_index > SLIM_LOG_BUFFER_SIZE) {
        slim_log_flush(log);
    }
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_log_hexdump(SlimLogContext log, void* data, u32_t length, u32_t stride, u8_t is_sparse)
{
    slim_log_printf(log, "\nHexdump (%d bytes):\n", length);

    // Print Header Row
    slim_log_printf(log, "ROW    \tBYTE\t");
    for (u8_t i = 0; i < stride; i++) {
        slim_log_printf(log, "0%x ", i);
    }
    slim_log_printf(log, "\n");

    // Print the Contents
    for (int row = 0; row < length; row++) {
        // Print Index and Byte Offset
        slim_log_printf(log, "%04d\t", row);
        slim_log_printf(log, "0x%04hhX\t", row * stride);

        // Print the Bytes
        for (int byte = stride - 1; byte >= 0; byte--) {
            u8_t value = ((u8_t*)data)[row * stride + byte];
            if (is_sparse && value == 0) {
                slim_log_printf(log, ".. ");
            } else {
                slim_log_printf(log, "%x ", value);
            }
        }

        slim_log_printf(log, "\n");
    }
    slim_log_printf(log, "\n");
}
// ---------------------------------------------------------------------------------------------------------------------