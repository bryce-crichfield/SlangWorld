#include <SlimBytecode.h>
#include <SlimData.h>

#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------------------------------------------------
struct SlimBytecodeData {
    u8_t* data;
    u32_t size;
};
// ---------------------------------------------------------------------------------------------------------------------
struct SlimBytecodeTable {
    SlimVector natives;
    SlimVector strings;
    SlimVector constants;
    SlimVector instructions;

    u32_t header_size;
    u32_t native_size;
    u32_t string_size;
    u32_t constant_size;
    u32_t instruction_size;

    u32_t header_offset;
    u32_t native_offset;
    u32_t string_offset;
    u32_t constant_offset;
    u32_t instruction_offset;
};
// ---------------------------------------------------------------------------------------------------------------------
SlimBytecodeData slim_bytecode_data_create(u8_t* data, u32_t size)
{
    SlimBytecodeData bytecode = malloc(sizeof(struct SlimBytecodeData));

    bytecode->data = malloc(size);
    bytecode->size = size;

    for (u32_t i = 0; i < size; i++) {
        bytecode->data[i] = data[i];
    }

    return bytecode;
}
// ---------------------------------------------------------------------------------------------------------------------
void slim_bytecode_data_destroy(SlimBytecodeData bytecode)
{
    if (bytecode == NULL) return;

    free(bytecode->data);
    free(bytecode);
    bytecode = NULL;
}
// ---------------------------------------------------------------------------------------------------------------------
SlimBytecodeTable slim_bytecode_table_create()
{
    SlimBytecodeTable table = malloc(sizeof(struct SlimBytecodeTable));

    table->natives = slim_vector_create(sizeof(char*));
    table->strings = slim_vector_create(sizeof(char*));
    table->constants = slim_vector_create(sizeof(u64_t));
    table->instructions = slim_vector_create(sizeof(SlimBytecodeInstruction));

    return table;
}
// ---------------------------------------------------------------------------------------------------------------------
void __char_destroy(void* data) { free(*(char**)data); }

void slim_bytecode_table_destroy(SlimBytecodeTable table)
{
    if (table == NULL) return;

    slim_vector_destroy(table->natives, __char_destroy);
    slim_vector_destroy(table->strings, __char_destroy);
    slim_vector_destroy(table->constants, NULL);
    slim_vector_destroy(table->instructions, NULL);

    free(table);
    table = NULL;
}

u16_t ___slim_u16_t_reverse(u16_t value) { return (value & 0x00FF) << 8 | (value & 0xFF00) >> 8; }

u32_t ___slim_u32_t_reverse(u32_t value)
{
    return (value & 0x000000FF) << 24 | (value & 0x0000FF00) << 8 | (value & 0x00FF0000) >> 8 |
           (value & 0xFF000000) >> 24;
}

// Data -> Table
SlimError slim_bytecode_table_load_data_header(SlimBytecodeTable table, SlimBytecodeData data)
{
    if (data->size < 32) return SLIM_ERROR;

    u32_t position = 0;

    // @Endianess
    memcpy(&table->header_size, data->data + position, sizeof(table->header_size));
    position += sizeof(table->header_size);
    table->header_size = ___slim_u32_t_reverse(table->header_size);

    memcpy(&table->native_size, data->data + position, sizeof(table->native_size));
    position += sizeof(table->native_size);
    table->native_size = ___slim_u32_t_reverse(table->native_size);

    memcpy(&table->string_size, data->data + position, sizeof(table->string_size));
    position += sizeof(table->string_size);
    table->string_size = ___slim_u32_t_reverse(table->string_size);

    memcpy(&table->constant_size, data->data + position, sizeof(table->constant_size));
    position += sizeof(table->constant_size);
    table->constant_size = ___slim_u32_t_reverse(table->constant_size);

    memcpy(&table->instruction_size, data->data + position, sizeof(table->instruction_size));
    position += sizeof(table->instruction_size);
    table->instruction_size = ___slim_u32_t_reverse(table->instruction_size);

    table->header_offset = 0;
    table->native_offset = table->header_size;
    table->string_offset = table->native_offset + table->native_size;
    table->constant_offset = table->string_offset + table->string_size;
    table->instruction_offset = table->constant_offset + table->constant_size;

    return SL_ERROR_NONE;
}

SlimError slim_bytecode_table_load_data_native(SlimBytecodeTable table, SlimBytecodeData data)
{
    // Each entry in the native table is composed of a 2 byte length followed by the null-terminated string.

    u32_t position = table->native_offset;

    while (position < table->string_offset) {
        u16_t length;
        memcpy(&length, data->data + position, sizeof(length));
        position += sizeof(length);
        length = ___slim_u16_t_reverse(length);

        char* native = malloc(length + 1);
        memcpy(native, data->data + position, length);
        native[length] = '\0';
        position += length;

        // This simple stores the start of the string in memory
        slim_vector_append(table->natives, &native);
    }

    return SL_ERROR_NONE;
}

SlimError slim_bytecode_table_load_data_string(SlimBytecodeTable table, SlimBytecodeData data) { return SL_ERROR_NONE; }

SlimError slim_bytecode_table_load_data_constant(SlimBytecodeTable table, SlimBytecodeData data)
{
    return SL_ERROR_NONE;
}

SlimError slim_bytecode_table_load_data_instruction(SlimBytecodeTable table, SlimBytecodeData data)
{
    return SL_ERROR_NONE;
}

SlimError slim_bytecode_table_load_data(SlimBytecodeTable table, SlimBytecodeData data)
{
    SlimError error = SL_ERROR_NONE;

    error = slim_bytecode_table_load_data_header(table, data);
    if (error != SL_ERROR_NONE) return error;

    error = slim_bytecode_table_load_data_native(table, data);
    if (error != SL_ERROR_NONE) return error;

    error = slim_bytecode_table_load_data_string(table, data);
    if (error != SL_ERROR_NONE) return error;

    error = slim_bytecode_table_load_data_constant(table, data);
    if (error != SL_ERROR_NONE) return error;

    error = slim_bytecode_table_load_data_instruction(table, data);
    if (error != SL_ERROR_NONE) return error;

    return error;
}

// Accessors
u32_t slim_bytecode_table_get_size_header(SlimBytecodeTable table) { return table->header_size; }
u32_t slim_bytecode_table_get_size_natives(SlimBytecodeTable table) { return table->native_size; }
u32_t slim_bytecode_table_get_size_strings(SlimBytecodeTable table) { return table->string_size; }
u32_t slim_bytecode_table_get_size_constants(SlimBytecodeTable table) { return table->constant_size; }
u32_t slim_bytecode_table_get_size_instrs(SlimBytecodeTable table) { return table->instruction_size; }

u32_t slim_bytecode_table_get_offset_header(SlimBytecodeTable table) { return table->header_offset; }
u32_t slim_bytecode_table_get_offset_natives(SlimBytecodeTable table) { return table->native_offset; }
u32_t slim_bytecode_table_get_offset_strings(SlimBytecodeTable table) { return table->string_offset; }
u32_t slim_bytecode_table_get_offset_constants(SlimBytecodeTable table) { return table->constant_offset; }
u32_t slim_bytecode_table_get_offset_instrs(SlimBytecodeTable table) { return table->instruction_offset; }

SlimError slim_bytecode_table_lookup_native(SlimBytecodeTable table, u64_t index, char** string)
{
    if (index >= slim_vector_size(table->natives)) return SLIM_ERROR;

    char* accessed;
    slim_vector_access(table->natives, index, &accessed);

    printf("Native: %s\n", accessed);

    *string = accessed;

    return SL_ERROR_NONE;
}

SlimError slim_bytecode_table_lookup_string(SlimBytecodeTable table, u64_t index, char** string)
{
    if (index >= slim_vector_size(table->strings)) return SLIM_ERROR;

    char* accessed;
    slim_vector_access(table->strings, index, &accessed);

    printf("Native: %s\n", accessed);

    *string = accessed;

    return SL_ERROR_NONE;
}

SlimError slim_bytecode_table_lookup_constant(SlimBytecodeTable table, u64_t index, u64_t* constant)
{
    return SL_ERROR_NONE;
}

SlimError slim_bytecode_table_lookup_instruction(SlimBytecodeTable table, u64_t index, SlimBytecodeInstruction* instr)
{
    return SL_ERROR_NONE;
}