#include <iostream>
#include <string>
#include <fstream>
#include <conio.h>

#include "asm_functions.h"

int main(int argc, char** argv) {
    static const char* input = argv[1];

    if (argc == 1) {
        printf("main(): Input an assembly file to process.\n");
        system("pause");
        return 1;
    }
    else {
        std::fstream asm_src;
        FILE* asm_out = fopen("processed.asm", "wb");
        asm_src.open(input, std::ios::in);
        if (asm_src.is_open() && asm_out != NULL) {
            fprintf(asm_out, "[YourPatchNamehere]\nmoduleMatches = 0x6267bfd0\n.origin = codecave\n\nentry_address = 0x2d5b828\n\n%s\n", return_func);
            fprintf(asm_out, "entry_point:\n%s\nbla main ; Call user code.\n%s\n", save_registers_func, restore_registers_func);
            std::string line;
            while (getline(asm_src, line)) {
                // Remove compiler clutter
                if (line.find("\t.file") == 0 || line.find("\t.text") == 0 || line.find("\t.globl") == 0 ||
                    line.find("\t.type") == 0 || line.find("\t.size") == 0 || line.find("\t.section") == 0 ||
                    line.find("\t.weak") == 0 || line.find("\t.data") == 0 || line.find("\t.ident") == 0 ||
                    line.find("\t.addrsig") == 0 || line.find("\t.addrsig_sym") == 0 || line.find("\t.machine") == 0 ||
                    line.find("\t.gnu_attribute") == 0 || line.find(".lcomm") == 0 || line.find("\t.align") == 0 ||
                    line.find("\tbl __eabi") == 0)
                {
                    continue; // Skip this line
                }

                // Replace GCC's weird "%r#" register notation
                while (line.find("%r") != -1) {
                    size_t pos = line.find("%r");
                    if (pos != -1) {
                        line.replace(pos, 2, "r");
                    }
                }
                while (line.find("%cr") != -1) {
                    size_t pos = line.find("%cr");
                    if (pos != -1) {
                        line.replace(pos, 3, "cr");
                    }
                }

                // Print to file
                fwrite(line.c_str(), line.size(), 1, asm_out);
                fprintf(asm_out, "\n");
            }
            fprintf(asm_out, "\n\n.origin = entry_address\nentry: ; This branch instruction replaces the original instruction.\nb entry_point\nreturn_address: ; Branching to this label will return to vanilla code.\n");
            fclose(asm_out);
            asm_src.close();
        }
    }
}
