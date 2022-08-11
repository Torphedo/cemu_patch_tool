# PPC Patch Compiler

This is a simple tool that will automatically compile C or C++ source code into PowerPC assembly, in a Cemu-friendly format. This program doesn't include a compiler, it
just automatically calls G++ from a devkitPPC install to generate assembly code and cleans it up for Cemu.

## Usage

You must install devkitPro (specifically, the `devkitPPC` module) to use this tool. It should be installed to `C:\devkitPro\devkitPPC` (or change the path in the source code
to your devkitPro path). Once devkitPro is installed, drag & drop a C/C++ file onto `PPC-Patch-Compiler.exe` to generate an `output.asm` file.
