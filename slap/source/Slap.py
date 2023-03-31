import struct
import sys
import os

from antlr4 import *
from typing import Optional
from dataclasses import dataclass

from parsing.SlapLexer import SlapLexer
from parsing.SlapParser import SlapParser
from parsing.SlapListener import SlapListener

from SlapLog import *
from assembler.SlapAssembler import SlapAssembler

from symbol.SlapSymbol import *
from symbol.SlapSymbolResolver import *
from symbol.SlapSymbolTabulator import *
from symbol.SlapSymbolValidator import *


# ----------------------------------------------------------------------------------------------------------------------
def parse_file(file_name) -> Optional[SlapParser.ProgramContext]:
    # Check for existence of file
    abspath = os.path.abspath(file_name)
    if not os.path.exists(abspath):
        log_error(f"File '{abspath}' does not exist")
        return None

    log_info(f"Parsing file {abspath}")

    input = FileStream(file_name)
    lexer = SlapLexer(input)
    stream = CommonTokenStream(lexer)
    parser = SlapParser(stream)
    tree = parser.program()

    # Check for errrors and possibly exit
    if parser.getNumberOfSyntaxErrors() > 0:
        log_error("Syntax errors detected")
        return None

    return tree


# ----------------------------------------------------------------------------------------------------------------------
def assemble(tree: SlapParser.ProgramContext) -> Optional[bytearray]:
    log_info("Assembling file")

    walker = ParseTreeWalker()

    tabulator = SlapSymbolTabulator()
    walker.walk(tabulator, tree)
    symbol_table = tabulator.symbol_table

    validator = SlapSymbolValidator(symbol_table)
    if not validator.isValid:
        log_error("Symbol validation failed (pre-pass)")
        return None

    walker.walk(validator, tree)
    if not validator.isValid:
        log_error("Symbol validation failed (post-pass)")
        return None

    resolver = SlapSymbolResolver(symbol_table)
    walker.walk(resolver, tree)

    assembler = SlapAssembler(symbol_table)
    walker.walk(assembler, tree)
    return assembler.build()


# ----------------------------------------------------------------------------------------------------------------------
def hexdump_bytecode(byte_array: bytearray):
    for i in range(0, len(byte_array), 9):
        bytes = byte_array[i : i + 9]
        print(f"{i:04x} {bytes.hex()}")


# ----------------------------------------------------------------------------------------------------------------------
def write_bytecode(byte_array: bytearray, file_name: str):
    with open(file_name, "wb") as f:
        f.write(byte_array)


# ----------------------------------------------------------------------------------------------------------------------
def main():
    if len(sys.argv) != 3:
        print("Usage: slap <input file> <output file>")
        return

    tree = parse_file(sys.argv[1])
    if tree is None:
        return

    bytecode = assemble(tree)
    if bytecode is None:
        return

    write_bytecode(bytecode, sys.argv[2])

    hexdump_bytecode(bytecode)


# ----------------------------------------------------------------------------------------------------------------------
if __name__ == "__main__":
    main()
