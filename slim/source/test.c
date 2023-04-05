// #include "SlimPlatform.h"
// #include <SlimBytecode.h>
// #include <stdio.h>

// int main(int argc, char** argv)
// {
//     // slim_platform_init(argc, argv);
//     // do {
//     //     slim_platform_update();
//     // } while (1);
//     // slim_platform_exit();
//     // return 0;
//     SlimError error;

//     SlimBytecodeFile file;
//     error = slim_bytecode_file_create(&file, "test.bin");
//     if (error != SL_ERROR_NONE) {
//         printf("Error: %d", error);
//         return 1;
//     }

//     SlimBytecode bytecode;
//     error = slim_bytecode_create(&bytecode, &file);
//     if (error != SL_ERROR_NONE) {
//         printf("Error: %d", error);
//         return 1;
//     }

//     const u8_t* native_table;
//     u32_t native_table_size;

//     error = slim_bytecode_section_native_get(&bytecode, &native_table, &native_table_size);
//     if (error != SL_ERROR_NONE) {
//         printf("Error: %d", error);
//         return 1;
//     }

//     printf("Native table size: %d\n", native_table_size);

//     for (u32_t i = 0; i < native_table_size; i++) {
//         printf("%d ", native_table[i]);
//     }

//     printf("\n");

//     slim_bytecode_destroy(&bytecode);

//     slim_bytecode_file_destroy(&file);

//     return 0;
// }