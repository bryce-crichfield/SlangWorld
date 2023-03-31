import struct
import sys
import os

from antlr4 import *
from typing import Optional
from dataclasses import dataclass

from parsing.SlapLexer import SlapLexer
from parsing.SlapParser import SlapParser
from parsing.SlapListener import SlapListener

from log.SlapLog import *
from assembler.SlapAssembler import SlapAssembler
from assembler.SlapByteWriter import SlapByteWriter

from symbol.SlapSymbol import *
from symbol.SlapSymbolResolver import *
from symbol.SlapSymbolTabulator import *
from symbol.SlapSymbolValidator import *

# ----------------------------------------------------------------------------------------------------------------------
class SlapProgram:
    def __init__(self, file_name):
        self.file_name = file_name
        self.tree = None

    # ------------------------------------------------------------------------------------------------------------------
    def parse(self) -> bool:
        # Check for existence of file
        abspath = os.path.abspath(self.file_name)
        self.abspath = abspath
        if not os.path.exists(abspath):
            error(f"File '{abspath}' does not exist")
            return False

        info(f"Parsing file {abspath}")

        input = FileStream(self.file_name)
        lexer = SlapLexer(input)
        stream = CommonTokenStream(lexer)
        parser = SlapParser(stream)
        tree = parser.program()

        # Check for errrors and possibly exit
        if parser.getNumberOfSyntaxErrors() > 0:
            error("Syntax errors detected")
            return False

        self.tree = tree
        return True

    # ------------------------------------------------------------------------------------------------------------------
    def assemble(self) -> bool:
        info("Assembling file")

        walker = ParseTreeWalker()

        tabulator = SlapSymbolTabulator()
        walker.walk(tabulator, self.tree)
        symbol_table = tabulator.symbol_table

        validator = SlapSymbolValidator(symbol_table)
        if not validator.isValid:
            error("Symbol validation failed (pre-pass)")
            return False

        walker.walk(validator, self.tree)
        if not validator.isValid:
            error("Symbol validation failed (post-pass)")
            return False

        resolver = SlapSymbolResolver(symbol_table)
        walker.walk(resolver, self.tree)

        assembler = SlapAssembler(symbol_table)
        walker.walk(assembler, self.tree)
        self.byte_array = assembler.byte_array
        return True

    # ------------------------------------------------------------------------------------------------------------------
    def output(self):
        info("Writing output file")
        # Using the input path, replace the extension with .slap
        output_path = os.path.splitext(self.abspath)[0] + ".slim"
        with open(output_path, "wb") as f:
            f.write(self.byte_array)
        # TODO: HEXDUMP
