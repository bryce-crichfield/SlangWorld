#include <SlimFile.h>

#include <stdlib.h>
#include <stdio.h>

SlimError slim_file_read(const char* filename, u8_t **data, u32_t *size) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        return SLIM_ERROR;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);

    *data = malloc(*size);
    fseek(file, 0, SEEK_SET);

    fread(*data, *size, 1, file);

    fclose(file);

    return SL_ERROR_NONE;
}