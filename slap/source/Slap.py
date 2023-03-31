import sys

from assembler.SlapProgram import SlapProgram
from log import SlapLog as log
# ----------------------------------------------------------------------------------------------------------------------
def main():
    log.init("slap.log", True)

    if len(sys.argv) != 2:
        print("Usage: slap <input file>")
        return

    program = SlapProgram(sys.argv[1])
    if not program.parse():
        return
    if not program.assemble():
        return
    program.output()
    
    log.flush()

# ----------------------------------------------------------------------------------------------------------------------
if __name__ == "__main__":
    main()
