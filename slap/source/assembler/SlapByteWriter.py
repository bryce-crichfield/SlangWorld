import struct

class SlapByteWriter:
    """Used by the assembler to write various contructs such as numbers, floats, and strings in bytes"""

    def __init__(self, byte_array: bytearray, endianness: str):
        self.byte_array = byte_array
        self.endianness = endianness

    def write_byte(self, byte: int):
        self.byte_array.append(byte)

    def write_number_little_endian(self, value: int):
        self.write_byte(value & 0xFF)
        self.write_byte((value >> 8) & 0xFF)
        self.write_byte((value >> 16) & 0xFF)
        self.write_byte((value >> 24) & 0xFF)
        self.write_byte((value >> 32) & 0xFF)
        self.write_byte((value >> 40) & 0xFF)
        self.write_byte((value >> 48) & 0xFF)
        self.write_byte((value >> 56) & 0xFF)
        
    def write_number_big_endian(self, value: int):
        self.write_byte((value >> 56) & 0xFF)
        self.write_byte((value >> 48) & 0xFF)
        self.write_byte((value >> 40) & 0xFF)
        self.write_byte((value >> 32) & 0xFF)
        self.write_byte((value >> 24) & 0xFF)
        self.write_byte((value >> 16) & 0xFF)
        self.write_byte((value >> 8) & 0xFF)
        self.write_byte(value & 0xFF)

    def write_number(self, value: int):
        if self.endianness == "little":
            self.write_number_little_endian(value)
        elif self.endianness == "big":
            self.write_number_big_endian(value)
        else:
            raise Exception(f"Unknown endianness '{self.endianness}'")

    def write_argless_opcode(self, opcode: int):
        self.write_byte(opcode)
        self.write_number(0)

    def write_bin(self, binary: str):
        value: int = int(binary[2:], 2)
        self.write_number(value)

    def write_hex(self, hex: str):
        value: int = int(hex[2:], 16)
        self.write_number(value)        

    def write_int(self, integer: int):
        value: int = int(integer[2:], 10)
        self.write_number(value)

    def write_float(self, string: str):
        # Writes float as ieee 754 double
        as_float = float(string[2:])
        value: int = struct.unpack("Q", struct.pack("d", as_float))[0]
        self.write_number(value)      
        