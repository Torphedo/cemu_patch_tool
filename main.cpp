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
            fprintf(asm_out, "[YourPatchNamehere]\nmoduleMatches = 0x6267bfd0\n.origin = codecave\n\n");
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

                // Replace the "." in front of ".LC0:" and other GCC-generated labels for local variables.
                // Cemu doesn't like them and tries to interpret as an assembler directive/macro.
                while (line.find(".L") != -1) {
                    size_t pos = line.find(".L");
                    if (pos != -1) {
                        line.replace(pos, 2, "L");
                    }
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

                // I'm sorry for this code...it's really janky, but it does the job.
                // See Appendix B.9 Miscellaneous Mnemonics in this PDF:
                // https://arcb.csc.ncsu.edu/~mueller/cluster/ps3/SDK3.0/docs/arch/PPC_Vers202_Book1_public.pdf
                if (line.find("\tla ") == 0) {
                    // Replace "la" with "addi"
                    line.replace(0, 3, "\taddi");

                    size_t label_pos = line.find(',');
                    size_t ry_pos = line.find('(');
                    size_t ry_end = line.find(')');
                    size_t label_length = ry_pos - label_pos;
                    size_t ry_length = ry_end - ry_pos;

                    char* buffer = (char*)calloc(1, label_length + ry_length);

                    strncpy(buffer, &line.c_str()[ry_pos + 1], ry_length - 1);
                    strncat(buffer, &line.c_str()[label_pos], label_length);

                    line.replace(label_pos + 1, label_length + ry_length, buffer);
                    free(buffer);
                }

                // Print to file
                fwrite(line.c_str(), line.size(), 1, asm_out);
                fprintf(asm_out, "\n");
            }
            fprintf(asm_out, "\n\n.origin = 0x2d5b828\nentry: ; This branch instruction replaces the original instruction.\nb entry_point\nreturn_address: ; Branching to this label will return to vanilla code.\n");
            fclose(asm_out);
            asm_src.close();
        }
    }
}
