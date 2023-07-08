# Cemu Patch Tool

This is a simple tool that cleans up PowerPC assembly output from GCC and puts
it in a Cemu-friendly format. It also inserts setup and teardown code so that
your C/C++ code won't impact the CPU state after it returns to game code.

## Usage
```
cemu_patch_tool [ASM file] [patch module name] [hooking address]
```

### ASM File
This is the assembly outputted by the compiler. You'll need special compiler
flags to emit assembly and remove a bunch of the extra information Cemu doesn't
understand.

### Patch Module Name
This is just used to identify your patch in the Cemu debugger. Name it something
like `BOTW_YourModName`.

### Hooking Address
The instruction at this address will be replaced with a branch instruction. It
will save the CPU state onto the stack, run your code, then restore the CPU
state and go back to the original game code.

The tool will produce a `processed.asm` file as output.
