#include "slim.h"

// Test ----------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv) {
    SlimMachine* machine = slim_machine_create();
    slim_machine_clear(machine);

    if (argc < 2) {
        printf("No input file\n");
        return 1;
    }

    const char* input_file = argv[1];
    SlimBytecode* bytecode = slim_bytecode_load(input_file);
    if (bytecode == NULL) {
        printf("Failed to load bytecode\n");
        return 1;
    }

    slim_machine_load(machine, bytecode->data, bytecode->bytesize);
    slim_machine_launch(machine);
    slim_machine_dump_stack(machine);
    slim_machine_dump_registers(machine);
    slim_machine_dump_memory(machine);
}