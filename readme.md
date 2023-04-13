# PPC Patch Compiler

This is a simple tool that cleans up PowerPC assembly output from GCC and puts it in a Cemu-friendly format. The tool
also inserts entry and exit code to cleanly enter and exit from a hooked address.

## Usage
```
ppc_patch_compiler.exe [input .asm file]
```
The tool will produce a `processed.asm` file as output.
