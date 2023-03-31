from parsing.SlapListener import SlapListener
from parsing.SlapParser import SlapParser

from .SlapSymbol import *
from SlapLog import log_info


# ----------------------------------------------------------------------------------------------------------------------
class SlapSymbolResolver(SlapListener):
    """Uses the symbol table to resolve symbols in the parse tree to their final address. The final bytecode will only
    consist of the assemble section table, so native symbols must be resolved to their specified identifier.
    """

    def __init__(self, symbol_table: SlapSymbolTable):
        self.symbol_table = symbol_table
        self.symbol_table.calculate_addresses()

    def enterSectionSpecifier(self, ctx: SlapParser.SectionSpecifierContext):
        # If we are in the context of an instruction resolve, otherwise don't do anything
        if isinstance(ctx.parentCtx, SlapParser.SectionDeclarationContext):
            return

        if isinstance(ctx.parentCtx, SlapParser.NativeDeclarationContext):
            return

        name = ctx.LABEL().getText()
        address = 0

        for section in self.symbol_table.section_symbols:
            if section.name == name:
                address = section.address
                break

        for native in self.symbol_table.native_symbols:
            if native.name == name:
                address = native.identifier
                break
        
        hex_str = f"0x{address:08X}"
        log_info(f"Resolved symbol '{name}' to address {hex_str}")
        ctx.resolved_address = address