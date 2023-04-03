- [SLIM] Add proper cli and command facade 
- [SLIM] Right now integer arith is really only unsigned integer arith, we need to support or add full integer arith
- [SLIM] Add section based loading of bytecode
- [SLIM] Fix the native function table to be of type map[string -> function]
- [SLIM] Integrate DLL and SO loading of native code into the native function table
- [SLIM] We need a way to easily write validation tests for the results of the VM.
- [SLIM] We should add a more robust way for the itself machine to present output to the user.  This would help a lot in 
    debugging and testing.  This wouldn't sit at the level of the VM, but rather at the level of the platform and allow us
    to inspect the state of the VM.


- [SLAP] Add cast operation to grammar and assembler, and remove itof and ftoi
- [SLAP] Add header/metadata section writer 
- [SLAP] Add native symbol section writer

- [PROJECT] Define more robust tests for core validation
- [PROJECT] Add a proper build system like cmake or ninja, with our running and testing scripts integrated into it
