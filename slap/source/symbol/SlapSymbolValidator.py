from parsing.SlapListener import SlapListener
from parsing.SlapParser import SlapParser

from .SlapSymbol import SlapSymbolTable
from SlapLog import *


# ----------------------------------------------------------------------------------------------------------------------
class SlapSymbolValidator(SlapListener):
    """Walks the parse tree and validates that all symbols follow a number of rules:
    - The existence of a 'main' section
    - That there are no naming conflicts between sections and/or native symbols
    - That every reference to a symbol points to a valid symbol
    - That every reference to a symbol is of the correct type (e.g jump can't jump to a native symbol)
    """

    def __init__(self, symbol_table: SlapSymbolTable):
        self.symbol_table = symbol_table
        self.isValid = True

        self.isValid = self.isValid and self._validate_main()
        self.isValid = self.isValid and self._validate_name_uniqueness()

    def _validate_main(self) -> bool:
        hasMain = any(
            section.name == "main" for section in self.symbol_table.section_symbols
        )
        if hasMain:
            return True

        log_error("No main section found")

        return False

    def _validate_name_uniqueness(self) -> bool:
        name_occurences = {}

        for section in self.symbol_table.section_symbols:
            if section.name in name_occurences:
                name_occurences[section.name] += 1
            else:
                name_occurences[section.name] = 1

        for native in self.symbol_table.native_symbols:
            if native.name in name_occurences:
                name_occurences[native.name] += 1
            else:
                name_occurences[native.name] = 1

        for name, occurences in name_occurences.items():
            if occurences > 1:
                log_error(f"Duplicate symbol name '{name}'")
                return False

        return True

    def _validate_reference_name(self, ctx: SlapParser.SectionSpecifierContext) -> bool:
        # If we are in the context of an instruction, we need to check if the label exists
        if isinstance(ctx.parentCtx, SlapParser.SectionDeclarationContext):
            return True

        if isinstance(ctx.parentCtx, SlapParser.NativeDeclarationContext):
            return True

        name = ctx.LABEL().getText()

        isSection = any(
            section.name == name for section in self.symbol_table.section_symbols
        )
        if isSection:
            return True

        isNative = any(
            native.name == name for native in self.symbol_table.native_symbols
        )
        if isNative:
            return True

        log_error("Unknown symbol '{name}'")
        return False

    def _validate_reference_type(self, ctx: SlapParser.SectionSpecifierContext) -> bool:
        # If our specifier is native, we cannot be in the context of any branch instruction
        name = ctx.LABEL().getText()

        isNative = any(
            native.name == name for native in self.symbol_table.native_symbols
        )
        if not isNative:
            return True

        if isinstance(ctx.parentCtx, SlapParser.InstructionJmpContext):
            log_error(f"Cannot jump to native symbol '{name}'")
            return False

        if isinstance(ctx.parentCtx, SlapParser.InstructionJeqContext):
            log_error(f"Cannot jump to native symbol '{name}'")
            return False

        if isinstance(ctx.parentCtx, SlapParser.InstructionJneContext):
            log_error(f"Cannot jump to native symbol '{name}'")
            return False

        return True

    def enterSectionSpecifier(self, ctx: SlapParser.SectionSpecifierContext):
        self.isValid = self.isValid and self._validate_reference_name(ctx)
        self.isValid = self.isValid and self._validate_reference_type(ctx)
