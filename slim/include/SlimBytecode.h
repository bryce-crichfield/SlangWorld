#pragma once

#include <SlimType.h>

// Conceptually the bytecode is a set of tables.  Each table contains different information that the runtime might
// need to execute the program.  The tables are:
// 1. The Header Table - This table contains metadata about the bytecode.  It contains the byte layout of the various
// tables, as well information on each table. The header table always starts at the beginning of the bytecode, and is
// always a fixed size of 256 bytes.
// 2. The Native Table - NumId -> StringQualifier
// 3. The String Table - NumId -> String
// 4. The Constant Table - NumId -> Constant
// 5. The Instruction Table - SSA Instruction

typedef struct SlimBytecodeData* SlimBytecodeData;

typedef struct SlimBytecodeTable* SlimBytecodeTable;

typedef struct SlimBytecodeInstruction {
    u8_t opcode;
    u64_t operand;
} SlimBytecodeInstruction;

SlimBytecodeData slim_bytecode_data_create(u8_t *data, u32_t size);
void slim_bytecode_data_destroy(SlimBytecodeData bytecode);

SlimBytecodeTable slim_bytecode_table_create();
void slim_bytecode_table_destroy(SlimBytecodeTable table);

// Data -> Table
SlimError slim_bytecode_table_load_data_header(SlimBytecodeTable table, SlimBytecodeData data);
SlimError slim_bytecode_table_load_data_native(SlimBytecodeTable table, SlimBytecodeData data);
SlimError slim_bytecode_table_load_data_string(SlimBytecodeTable table, SlimBytecodeData data);
SlimError slim_bytecode_table_load_data_constant(SlimBytecodeTable table, SlimBytecodeData data);
SlimError slim_bytecode_table_load_data_instruction(SlimBytecodeTable table, SlimBytecodeData data);

SlimError slim_bytecode_table_load_data(SlimBytecodeTable table, SlimBytecodeData data);

// Table -> Data
SlimError slim_bytecode_data_load_table_header(SlimBytecodeData data, SlimBytecodeTable tables);
SlimError slim_bytecode_data_load_table_native(SlimBytecodeData data, SlimBytecodeTable tables);
SlimError slim_bytecode_data_load_table_string(SlimBytecodeData data, SlimBytecodeTable tables);
SlimError slim_bytecode_data_load_table_constant(SlimBytecodeData data, SlimBytecodeTable tables);
SlimError slim_bytecode_data_load_table_instruction(SlimBytecodeData data, SlimBytecodeTable tables);

// Accessors
u32_t slim_bytecode_table_get_header_size(SlimBytecodeTable table);
u32_t slim_bytecode_table_get_native_size(SlimBytecodeTable table);
u32_t slim_bytecode_table_get_string_size(SlimBytecodeTable table);
u32_t slim_bytecode_table_get_constant_size(SlimBytecodeTable table);
u32_t slim_bytecode_table_get_instruction_size(SlimBytecodeTable table);

u32_t slim_bytecode_table_get_header_offset(SlimBytecodeTable table);
u32_t slim_bytecode_table_get_native_offset(SlimBytecodeTable table);
u32_t slim_bytecode_table_get_string_offset(SlimBytecodeTable table);
u32_t slim_bytecode_table_get_constant_offset(SlimBytecodeTable table);
u32_t slim_bytecode_table_get_instruction_offset(SlimBytecodeTable table);

SlimError slim_bytecode_table_lookup_native(SlimBytecodeTable table, u64_t index, char** string);
SlimError slim_bytecode_table_lookup_string(SlimBytecodeTable table, u64_t index, char** string);
SlimError slim_bytecode_table_lookup_constant(SlimBytecodeTable table, u64_t index, u64_t* constant);
SlimError slim_bytecode_table_lookup_instruction(SlimBytecodeTable table, u64_t index, SlimBytecodeInstruction* instr);