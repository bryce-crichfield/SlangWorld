#pragma once

#include <SlimType.h>

// ---------------------------------------------------------------------------------------------------------------------
typedef struct SlimBytecode SlimBytecode;
typedef struct SlimBytecodeFile SlimBytecodeFile;

struct SlimBytecodeFile {
    u8_t* data;
    u32_t size;
};

struct SlimBytecode {
    const u8_t* data;
    u32_t size;

    u32_t native_offset;
    u32_t native_size;

    u32_t code_offset;
    u32_t code_size;
};
// ---------------------------------------------------------------------------------------------------------------------
SlimError slim_bytecode_file_create(SlimBytecodeFile* file, const char* path);
SlimError slim_bytecode_file_destroy(SlimBytecodeFile* file);
SlimError slim_bytecode_create(SlimBytecode* bytecode, SlimBytecodeFile* file);
SlimError slim_bytecode_destroy(SlimBytecode* bytecode);
SlimError slim_bytecode_section_native_get(SlimBytecode* bytecode, const u8_t** table, u32_t* size);
SlimError slim_bytecode_section_code_get(SlimBytecode* bytecode, const u8_t** code, u32_t* size);
// ---------------------------------------------------------------------------------------------------------------------