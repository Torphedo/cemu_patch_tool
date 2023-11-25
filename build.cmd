zig cc -g0 -o main.s -target powerpc-linux-gnueabi -S -O3 -mbig-endian -m32 -exceptions -fno-ident -s -mllvm -ppc-asm-full-reg-names main.c

./cemu_patch_tool main.s BOTW_CustomCombos 0x02d5b828 patch_main.asm
