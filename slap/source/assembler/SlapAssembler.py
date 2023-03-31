from parsing.SlapParser import SlapParser
from parsing.SlapListener import SlapListener

from symbol.SlapSymbol import SlapSymbolTable
from SlapUtil import SlapByteWriter
from SlapLog import log_info, log_error

# ----------------------------------------------------------------------------------------------------------------------
class SlapAssembler(SlapListener):
    """Assembles the program into a byte array"""

    def __init__(self, symbol_table: SlapSymbolTable):
        self.symbol_table = symbol_table
        self.byte_array = bytearray()
        self.writer = SlapByteWriter(self.byte_array, "big")

    def enterInstructionNoop(self, ctx: SlapParser.InstructionNoopContext):
        self.writer.write_argless_opcode(0x00)

    def enterInstructionHalt(self, ctx: SlapParser.InstructionHaltContext):
        self.writer.write_argless_opcode(0x01)

    def enterInstructionLoadi(self, ctx: SlapParser.InstructionLoadiContext):
        self.writer.write_byte(0x10)

    def enterInstructionLoadr(self, ctx: SlapParser.InstructionLoadrContext):
        # TODO: There might be register validation I forgot about
        self.writer.write_byte(0x11)

    def enterInstructionLoadm(self, ctx: SlapParser.InstructionLoadmContext):
        self.writer.write_byte(0x12)

    def enterInstructionDrop(self, ctx: SlapParser.InstructionDropContext):
        self.writer.write_argless_opcode(0x13)

    def enterInstructionStorer(self, ctx: SlapParser.InstructionStorerContext):
        # TODO: There might be register validation I forgot about
        self.writer.write_byte(0x14)

    def enterInstructionStorem(self, ctx: SlapParser.InstructionStoremContext):
        self.writer.write_byte(0x15)

    def enterInstructionDup(self, ctx: SlapParser.InstructionDupContext):
        self.writer.write_argless_opcode(0x20)

    def enterInstructionSwap(self, ctx: SlapParser.InstructionSwapContext):
        self.writer.write_argless_opcode(0x21)

    def enterInstructionRot(self, ctx: SlapParser.InstructionRotContext):
        self.writer.write_argless_opcode(0x22)

    def enterInstructionAdd(self, ctx: SlapParser.InstructionAddContext):
        self.writer.write_argless_opcode(0x30)

    def enterInstructionSub(self, ctx: SlapParser.InstructionSubContext):
        self.writer.write_argless_opcode(0x31)

    def enterInstructionMul(self, ctx: SlapParser.InstructionMulContext):
        self.writer.write_argless_opcode(0x32)

    def enterInstructionDiv(self, ctx: SlapParser.InstructionDivContext):
        self.writer.write_argless_opcode(0x33)

    def enterInstructionMod(self, ctx: SlapParser.InstructionModContext):
        self.writer.write_argless_opcode(0x34)

    def enterInstructionAddf(self, ctx: SlapParser.InstructionAddfContext):
        self.writer.write_argless_opcode(0x35)

    def enterInstructionSubf(self, ctx: SlapParser.InstructionSubfContext):
        self.writer.write_argless_opcode(0x36)

    def enterInstructionMulf(self, ctx: SlapParser.InstructionMulfContext):
        self.writer.write_argless_opcode(0x37)

    def enterInstructionDivf(self, ctx: SlapParser.InstructionDivfContext):
        self.writer.write_argless_opcode(0x38)

    def enterInstructionModf(self, ctx: SlapParser.InstructionModfContext):
        self.writer.write_argless_opcode(0x39)

    def enterInstructionAlloc(self, ctx: SlapParser.InstructionAllocContext):
        self.writer.write_byte(0x40)

    def enterInstructionFree(self, ctx: SlapParser.InstructionFreeContext):
        self.writer.write_argless_opcode(0x41)

    def enterInstructionJmp(self, ctx: SlapParser.InstructionJmpContext):
        self.writer.write_byte(0x50)

    def enterInstructionJne(self, ctx: SlapParser.InstructionJneContext):
        self.writer.write_byte(0x51)

    def enterInstructionJeq(self, ctx: SlapParser.InstructionJeqContext):
        self.writer.write_byte(0x52)

    def enterInstructionCall(self, ctx: SlapParser.InstructionContext):
        # Determine if we are native or section call
        opcode = 0x50
        specifier = ctx.sectionSpecifier()
        name = specifier.LABEL().getText()

        isNative = any(
            native.name == name for native in self.symbol_table.native_symbols
        )
        if isNative:
            opcode = 0x52

        self.writer.write_byte(opcode)
        self.writer.write_number(specifier.resolved_address)

    def enterInstructionRet(self, ctx: SlapParser.InstructionContext):
        self.writer.write_argless_opcode(0x53)

    def enterInstructionFtoi(self, ctx: SlapParser.InstructionContext):
        self.writer.write_argless_opcode(0x70)

    def enterInstructionItof(self, ctx: SlapParser.InstructionContext):
        self.writer.write_argless_opcode(0x71)

    def enterInstructionItoc(self, ctx: SlapParser.InstructionContext):
        self.writer.write_argless_opcode(0x72)

    def enterWholeNumber(self, ctx: SlapParser.WholeNumberContext):
        if ctx.HEX_NUMBER() is not None:
            self.writer.write_hex(ctx.HEX_NUMBER().getText())
        elif ctx.BIN_NUMBER() is not None:
            self.writer.write_bin(ctx.BIN_NUMBER().getText())
        elif ctx.INT_NUMBER() is not None:
            self.writer.write_int(ctx.INT_NUMBER().getText())
        else:
            log_error("Unknown number type")

    def enterFloatingNumber(self, ctx: SlapParser.FloatingNumberContext):
        self.writer.write_float(ctx.FLOAT_NUMBER().getText())

    def build(self) -> bytearray:
        return self.byte_array
