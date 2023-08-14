@echo off
powerpc-eabi-gcc-11.1.0 -S -O3 -mbig-endian -m32 -mno-eabi -ffreestanding -fno-toplevel-reorder -mregnames -mno-sdata -fno-exceptions -fno-asynchronous-unwind-tables -fno-function-sections -fno-ident -finhibit-size-directive -omain.asm main.c

rem              Input      Module Name   Hook Address    Output
cemu_patch_tool main.asm BOTW_CustomCombos 0x02d5b828 patch_main.asm
del main.asm

