from parsing.SlapListener import SlapListener
from parsing.SlapParser import SlapParser

from .SlapSymbol import *


# ----------------------------------------------------------------------------------------------------------------------
class SlapSymbolTabulator(SlapListener):
    """Walks the parse tree and builds a symbol table by iteratively adding symbols to the symbol table"""

    def __init__(self):
        self.symbol_table = SlapSymbolTable([], [])
        self.global_index = 0
        self.current_section_size = 0

    def exitSectionDeclaration(self, ctx: SlapParser.SectionDeclarationContext):
        name = ctx.sectionSpecifier().LABEL().getText()
        section = SlapSectionSymbol(name, self.global_index, self.current_section_size)
        self.symbol_table.section_symbols.append(section)
        self.global_index += 1
        self.current_section_size += 1

    def exitNativeDeclaration(self, ctx: SlapParser.NativeDeclarationContext):
        name = ctx.sectionSpecifier().LABEL().getText()
        identifier = int(ctx.HEX_NUMBER().getText(), 16)
        native = SlapNativeSymbol(name, identifier)
        self.symbol_table.native_symbols.append(native)

    def exitInstruction(self, ctx: SlapParser.InstructionContext):
        self.current_section_size += 1