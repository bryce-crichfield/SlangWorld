#include "slim.h"

void slim_debug_machine_dump_stack(SlimMachine* machine) {
    printf("Stack:\n");
    for (u32_t i = 0; i < SLIM_MACHINE_STACK_SIZE; i++) {
        printf("%d: %llu\n", i, machine->stack[i]);
    }

    printf("Stack Pointer: %llu\n", (s64_t)machine->stack_pointer);

    printf("\n");
}

void slim_debug_machine_dump_registers(SlimMachine* machine) {
    printf("Registers:\n");
    for (u32_t i = 0; i < SLIM_MACHINE_REGISTERS; i++) {
        printf("%d: %llu\n", i, machine->registers[i]);
    }

    printf("\n");
}

void slim_debug_machine_dump_memory(SlimMachine* machine) {
    printf("Memory:\n");
    for (u32_t i = 0; i < SLIM_MACHINE_MEMORY_SIZE; i++) {
        printf("%d: %llu\n", i, machine->memory[i]);
    }

    printf("\n");
}

// Test ----------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv) {
    slim_debug_init("test.log", 1);

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
    slim_debug_machine_dump_stack(machine);
    slim_debug_machine_dump_registers(machine);
    slim_debug_machine_dump_memory(machine);

    slim_debug_hexdump(machine->stack, SLIM_MACHINE_STACK_SIZE, sizeof(u64_t), 1);

    slim_debug_close();
}