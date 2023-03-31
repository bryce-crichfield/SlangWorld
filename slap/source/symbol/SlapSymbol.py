from typing import Optional
from dataclasses import dataclass


# ----------------------------------------------------------------------------------------------------------------------
@dataclass
class SlapSectionSymbol:
    name: str  # Name of the symbol (e.g. "main")
    global_index: int  # Index of this symbol in the global symbol table (technically redundant)
    instruction_count: int  # How many instructions are in this section
    address: int = 0  # Address of this section in the final binary


# ----------------------------------------------------------------------------------------------------------------------
@dataclass
class SlapNativeSymbol:
    name: str  # Name of the symbol (e.g. "std.putch")
    identifier: int  # Identifier of the symbol (e.g. 0x0001), used to identify the symbol in the binary
    address: int = 0  # Address of this symbol in the final binary


# ----------------------------------------------------------------------------------------------------------------------
@dataclass
class SlapSymbolTable:
    section_symbols: list[SlapSectionSymbol]
    native_symbols: list[SlapNativeSymbol]

    def calculate_addresses(self):
        """Calculates the addresses of all symbols in the symbol table"""
        current_address = 0
        print(self.section_symbols)
        for section in self.section_symbols:
            section.address = current_address
            current_address += section.instruction_count * 9



