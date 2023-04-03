#include <SlimLog.h>

#include <stdio.h>
#include <stdlib.h>

// Debugging -----------------------------------------------------------------------------------------------------------
typedef struct SlimLogContext {
    const char* output_path;
    char buffer[SLIM_LOG_BUFFER_SIZE];
    u16_t buffer_index;
    u8_t writes_stdout;
} SlimLogContext;
// ---------------------------------------------------------------------------------------------------------------------
static SlimLogContext* slim_log_context = NULL;
// ---------------------------------------------------------------------------------------------------------------------
void slim_log_init(const char* output_path, u8_t writes_stdout)
{
    slim_log_context = malloc(sizeof(SlimLogContext));
    slim_log_context->output_path = output_path;
    slim_log_context->buffer_index = 0;
    slim_log_context->writes_stdout = writes_stdout;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_log_flush()
{
    FILE* file = fopen(slim_log_context->output_path, "a");
    fwrite(slim_log_context->buffer, 1, slim_log_context->buffer_index, file);
    fclose(file);
    slim_log_context->buffer_index = 0;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_log_close()
{
    slim_log_flush();
    free(slim_log_context);
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_log_check_error()
{
    // TODO: Implement
    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_log_printf(const char* format, ...)
{
    if (slim_log_context == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    slim_log_context->buffer_index += vsprintf(slim_log_context->buffer + slim_log_context->buffer_index, format, args);
    va_end(args);

    if (slim_log_context->writes_stdout) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

    if (slim_log_context->buffer_index > SLIM_LOG_BUFFER_SIZE) {
        slim_log_flush();
    }
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_log_hexdump(void* data, u32_t length, u32_t stride, u8_t is_sparse)
{
    slim_log_printf("\nHexdump (%d bytes):\n", length);

    // Print Header Row
    slim_log_printf("ROW    \tBYTE\t");
    for (u8_t i = 0; i < stride; i++) {
        slim_log_printf("0%x ", i);
    }
    slim_log_printf("\n");

    // Print the Contents
    for (int row = 0; row < length; row++) {
        // Print Index and Byte Offset
        slim_log_printf("%04d\t", row);
        slim_log_printf("0x%04hhX\t", row * stride);

        // Print the Bytes
        for (int byte = stride - 1; byte >= 0; byte--) {
            u8_t value = ((u8_t*)data)[row * stride + byte];
            if (is_sparse && value == 0) {
                slim_log_printf(".. ");
            } else {
                slim_log_printf("%x ", value);
            }
        }

        slim_log_printf("\n");
    }
    slim_log_printf("\n");
}
// ---------------------------------------------------------------------------------------------------------------------