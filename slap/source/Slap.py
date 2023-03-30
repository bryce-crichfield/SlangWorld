from antlr4 import *
from parsing.SlapLexer import SlapLexer
from parsing.SlapParser import SlapParser
from parsing.SlapListener import SlapListener
import sys
from dataclasses import dataclass

# ----------------------------------------------------------------------------------------------------------------------
@dataclass
class OpcodeDefintion:
    name: str
    byte: int
    argument_count: int


OPCODE_TABLE = {}

def define_opcode(name, byte, argument_count):
    OPCODE_TABLE[name] = OpcodeDefintion(name, byte, argument_count)

def match_opcode(opcode: str) -> OpcodeDefintion:
    return OPCODE_TABLE[opcode.upper()]

define_opcode("NOOP", 0x00, 0)
define_opcode("HALT", 0x01, 0)

define_opcode("LOADI", 0x10, 1)
define_opcode("LOADR", 0x11, 1)
define_opcode("LOADM", 0x12, 1)
define_opcode("DROP", 0x13, 0)
define_opcode("STORER", 0x14, 1)
define_opcode("STOREM", 0x15, 0)

define_opcode("DUP", 0x20, 0)
define_opcode("SWAP", 0x21, 0)
define_opcode("ROT", 0x22, 0)

define_opcode("ADD", 0x30, 0)
define_opcode("SUB", 0x31, 0)
define_opcode("MUL", 0x32, 0)
define_opcode("DIV", 0x33, 0)
define_opcode("MOD", 0x34, 0)

define_opcode("ALLOC", 0x40, 1)
define_opcode("FREE", 0x41, 0)

define_opcode("JMP", 0x50, 1)
define_opcode("JNE", 0x51, 1)
define_opcode("JEQ", 0x52, 1)

define_opcode("CALL", 0x60, 1)
define_opcode("RET", 0x61, 0)
# ----------------------------------------------------------------------------------------------------------------------
@dataclass
class SectionRecord:
    global_index: int  # Which section is it?
    instruction_count: int  # How large is the section?
    global_byte_position: int  # Where does the section start?


# ----------------------------------------------------------------------------------------------------------------------
class SlapSymbolTabulator(SlapListener):
    """Tabulates section symbols, native symbols, etcs and produces a virtual symbol table [String -> Section]"""

    def __init__(self):
        self.symbol_table = {}
        self.current_sections_instruction_count = 0
        self.global_index = 0
        self.global_byte_position = 0

    def exitSection(self, ctx: SlapParser.SectionContext):
        section = SectionRecord(
            self.global_index,
            self.current_sections_instruction_count,
            self.global_byte_position,
        )
        name = ctx.LABEL().getText()
        self.symbol_table[name] = section
        self.global_index += 1
        self.current_sections_instruction_count = 0
        self.global_byte_position += 9 * section.instruction_count

    def exitInstruction(self, ctx: SlapParser.InstructionContext):
        self.current_sections_instruction_count += 1

# ----------------------------------------------------------------------------------------------------------------------
class SlapSymbolResolver(SlapListener):
    """Resolves named arguments in the instructions to their corresponding byte index from the virtual symbol table"""

    def __init__(self, symbol_table):
        self.symbol_table = symbol_table

    def enterArgument(self, ctx: SlapParser.ArgumentContext):
        if ctx.LABEL():
            name = ctx.LABEL().getText()
            section = self.symbol_table[name]
            ctx.byte_index = section.global_byte_position


# ----------------------------------------------------------------------------------------------------------------------
class SlapAssembler(SlapListener):
    """Assembles the program into a byte array by generating the byte code for each instruction"""

    def __init__(self, symbol_table):
        self.symbol_table = symbol_table
        self.byte_array = bytearray()

    def write_number(self, number):
        bytes = [0] * 8
        for i in range(8):
            bytes[i] = number & 0xFF
            number >>= 8

        # Reverse the bytes
        bytes.reverse()
        for byte in bytes:
            self.byte_array.append(byte)

    def enterInstruction(self, ctx: SlapParser.InstructionContext):
        opcode = ctx.OPCODE().getText()
        opcode_def = match_opcode(opcode)
        if opcode_def.argument_count != 0 and ctx.argument() is None:
            raise Exception("Invalid argument count")

        self.byte_array.append(opcode_def.byte)
        if ctx.argument() is None:
            for i in range(8):
                self.byte_array.append(0)

    def enterArgument(self, ctx: SlapParser.ArgumentContext):
        if ctx.LABEL():
            byte_index = ctx.byte_index
            self.write_number(byte_index)

        elif ctx.NUMBER():
            string = ctx.NUMBER().getText()
            number = 0

            if string.startswith("0x"):
                number = int(string[2:], 16)
            elif string.startswith("0b"):
                number = int(string[2:], 2)
            elif string.startswith("0f"):
                number = float(string[2:])
            else:
                number = int(string)

            self.write_number(number)
        else:
            raise Exception("Invalid argument")


# ----------------------------------------------------------------------------------------------------------------------
def main():
    if len(sys.argv) != 3:
        print("Usage: slap <input file> <output file>")
        return

    input = FileStream(sys.argv[1])
    lexer = SlapLexer(input)
    stream = CommonTokenStream(lexer)
    parser = SlapParser(stream)
    tree = parser.program()
    walker = ParseTreeWalker()

    # Perform the necessary passes to assemble the program
    symbolTabulator = SlapSymbolTabulator()
    walker.walk(symbolTabulator, tree)

    symbolResolver = SlapSymbolResolver(symbolTabulator.symbol_table)
    walker.walk(symbolResolver, tree)

    assembler = SlapAssembler(symbolTabulator.symbol_table)
    walker.walk(assembler, tree)

    # Hex dump the assembled program
    for i in range(0, len(assembler.byte_array), 9):
        bytes = assembler.byte_array[i : i + 9]
        print(f"{i:04x} {bytes.hex()}")

    # Write the assembled program to a file
    with open(sys.argv[2], "wb") as f:
        f.write(assembler.byte_array)
# ----------------------------------------------------------------------------------------------------------------------
if __name__ == "__main__":
    main()
