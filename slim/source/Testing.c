#include <SlimPlatform.h>
#include <SlimBytecode.h>
#include <SlimData.h>
#include <SlimFile.h>

#include <assert.h>
#include <stdio.h>

void testSlimArray()
{
    SlimVector vector = slim_vector_create(sizeof(u32_t));

    u32_t element = 0;
    slim_vector_insert(vector, 0, &element);

    element = 1;
    slim_vector_insert(vector, 0, &element);

    element = 2;
    slim_vector_insert(vector, 0, &element);

    slim_vector_remove(vector, 1, &element);
    assert(element == 1);

    slim_vector_remove(vector, 0, &element);
    assert(element == 2);

    slim_vector_access(vector, 0, &element);
    assert(element == 0);

    slim_vector_remove(vector, 0, &element);
    assert(element == 0);

    slim_vector_destroy(vector, NULL);
}

void testFileLoading()
{
    // TODO: IMPLEMENT FILE LOADING TEST
}

void testBytecode()
{
    const u8_t BYTECODE_SIZE = 32 + 8;
    // clang-format off
    u8_t file_data[BYTECODE_SIZE] = {
        0x00, 0x00, 0x00, 0x20, // header_size
        0x00, 0x00, 0x00, 0x08, // native_size
        0x00, 0x00, 0x00, 0x00, // string_size
        0x00, 0x00, 0x00, 0x00, // constant_size
        0x00, 0x00, 0x00, 0x00, // instruction_size
        0x00, 0x00, 0x00, 0x00, // padding
        0x00, 0x00, 0x00, 0x00, // padding
        0x00, 0x00, 0x00, 0x00, // padding

        0x00, 0x06, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x00, // native: 0    'a'
    };
    // clang-format on

    SlimBytecodeData bytecode = slim_bytecode_data_create(file_data, BYTECODE_SIZE);

    SlimBytecodeTable table = slim_bytecode_table_create();

    SlimError error = slim_bytecode_table_load_data(table, bytecode);
    if (error != SL_ERROR_NONE) {
        printf("Error\n");
        goto exit;
    }

    u32_t header_size = slim_bytecode_table_get_size_header(table);
    printf("header_size: %d\n", header_size);

    u32_t native_size = slim_bytecode_table_get_size_natives(table);
    printf("native_size: %d\n", native_size);

    u32_t string_size = slim_bytecode_table_get_size_strings(table);
    printf("string_size: %d\n", string_size);

    u32_t constant_size = slim_bytecode_table_get_size_constants(table);
    printf("constant_size: %d\n", constant_size);

    u32_t instruction_size = slim_bytecode_table_get_size_instrs(table);
    printf("instruction_size: %d\n", instruction_size);

    u32_t header_offset = slim_bytecode_table_get_offset_header(table);
    printf("header_offset: %d\n", header_offset);

    u32_t native_offset = slim_bytecode_table_get_offset_natives(table);
    printf("native_offset: %d\n", native_offset);

    u32_t string_offset = slim_bytecode_table_get_offset_strings(table);
    printf("string_offset: %d\n", string_offset);

    u32_t constant_offset = slim_bytecode_table_get_offset_constants(table);
    printf("constant_offset: %d\n", constant_offset);

    u32_t instruction_offset = slim_bytecode_table_get_offset_instrs(table);
    printf("instruction_offset: %d\n", instruction_offset);

    char* string;
    error = slim_bytecode_table_lookup_native(table, 0, &string);
    if (error != SL_ERROR_NONE) {
        printf("Error\n");
        goto exit;
    }

    printf("string length: %d\n", strlen(string));
    printf("string: %s\n", string);

exit:
    slim_bytecode_data_destroy(bytecode);
    slim_bytecode_table_destroy(table);

    return;
}

void testPlatform(int argc, char** argv) {
    SlimPlatform platform = slim_platform_create(argc, argv);
    
    slim_platform_destroy(platform);
}

int main(int argc, char** argv)
{
    testSlimArray();
    testFileLoading();
    testBytecode();
    testPlatform(argc, argv);
    return 0;
}