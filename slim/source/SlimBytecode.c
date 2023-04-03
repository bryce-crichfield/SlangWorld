#include <SlimBytecode.h>
#include <SlimLog.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_bytecode_file_create(SlimBytecodeFile* file, const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f) {
        return SLIM_ERROR;
    }

    fseek(f, 0, SEEK_END);
    file->size = ftell(f);

    file->data = (u8_t*)malloc(file->size);
    if (!file->data) {
        return SLIM_ERROR;
    }

    fseek(f, 0, SEEK_SET);

    fread((void*)file->data, 1, file->size, f);

    fclose(f);

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_bytecode_file_destroy(SlimBytecodeFile* file)
{
    if (file->data) {
        free((void*)file->data);
    }

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_bytecode_create(SlimBytecode* bytecode, SlimBytecodeFile* file)
{
    // Deep copy the bytecode file data into the bytecode struct
    bytecode->data = (u8_t*)malloc(file->size);
    if (!bytecode->data) {
        return SLIM_ERROR;
    }

    memcpy((void*)bytecode->data, file->data, file->size);

    bytecode->size = file->size;

    // Read the header, which contains the offsets and sizes of the various sections
    u32_t read_index = 0;

    bytecode->native_offset = *(u32_t*)(bytecode->data + read_index);
    read_index += sizeof(bytecode->native_offset);
    slim_log_info("native_offset: %d\n", bytecode->native_offset);

    bytecode->native_size = *(u32_t*)(bytecode->data + read_index);
    read_index += sizeof(bytecode->native_size);
    slim_log_info("native_size: %d\n", bytecode->native_size);

    bytecode->code_offset = *(u32_t*)(bytecode->data + read_index);
    read_index += sizeof(bytecode->code_offset);
    slim_log_info("code_offset: %d\n", bytecode->code_offset);

    bytecode->code_size = *(u32_t*)(bytecode->data + read_index);
    read_index += sizeof(bytecode->code_size);
    slim_log_info("code_size: %x\n", bytecode->code_size);

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_bytecode_destroy(SlimBytecode* bytecode)
{
    if (bytecode->data) {
        free((void*)bytecode->data);
    }

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_bytecode_section_native_get(SlimBytecode* bytecode, const u8_t** table, u32_t* size) 
{
    *table = bytecode->data + bytecode->native_offset;
    *size = bytecode->native_size;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_bytecode_section_code_get(SlimBytecode* bytecode, const u8_t** code, u32_t* size)
{
    *code = bytecode->data + bytecode->code_offset;
    *size = bytecode->code_size;

    return SL_ERROR_NONE;
}
// ---------------------------------------------------------------------------------------------------------------------