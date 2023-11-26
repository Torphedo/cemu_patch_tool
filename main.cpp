#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <string>
#include <fstream>

#include "logging.h"
#include "asm_functions.h"

// If we see any of these patterns, just skip to the next line.
static const char* skip_lines[] = {
    "\t.file", "\t.text", "\t.globl", "\t.type", "\t.size", "\t.section",
    "\t.weak", "\t.data", "\t.ident", "\t.addrsig", "\t.addrsig_sym",
    "\t.machine", "\t.gnu_attribute", ".lcomm", "\t.align", "\tbl __eabi",
    "\t.p2align", "\t.cfi_"
};

bool should_skip_line(const char* line) {
    // Tell caller to skip if we find any of the strings from the array
    uint32_t array_size = (sizeof(skip_lines) / sizeof(char*));
    for (uint32_t i = 0; i < array_size; i++) {
        if (strstr(line, skip_lines[i]) != NULL) {
            return true; // Skip if we found a match
        }
    }
    return false; // No match.
}

int main(int argc, char** argv) {
    static const char* input = argv[1];
    static const char* module_name = argv[2];
    static const char* hook_addr = argv[3];
    static const char* output_filename = argv[4];
    
    // Errors, info, and instructions for new users.
    if (argc < 3) {
        LOG_MSG(error, "Not enough arguments provided.\n");
        
        LOG_MSG(info, "CLI usage:\n");
        printf("\tcemu_patch_tool [ASM file] [patch module name] [hooking address] [output filename]\n");
        
        LOG_MSG(info, "The ASM file is the output from your compiler.\n");
        LOG_MSG(info, "The module name is used to find your patch in the Cemu debugger.\n");
        LOG_MSG(info, "The hooking address is the address that will be forced to call your main() function.\n");
        printf("\tAll registers are saved before your code runs and restored afterwards.\n");
        printf("\tThe address must be 7-8 digits of hexidecimal, like \"0x02d5b828\" or \"0x2d5b828\".\n");
        LOG_MSG(info, "The output file is the final patch filename.\n");
        return 1;
    }

    // All arguments were provided, we can continue
    std::fstream asm_src;
    FILE* asm_out = fopen(output_filename, "wb");
    asm_src.open(input, std::ios::in);

    // Error checking
    if (!asm_src.is_open() || asm_out == NULL) {
        free(asm_out);
        asm_src.close();
    }

    fprintf(asm_out, "[%s]\n", module_name);
    fprintf(asm_out, "moduleMatches = 0x6267bfd0\n.origin = codecave\n\n");
    fprintf(asm_out, "; Assembly auto-generated with Zig and filtered through Torph's Cemu patch tool.\n\n");
    
    /* The entry_point label saves all registers to the stack, calls the user's
     * main() function, then reloads all register data from the stack.
     */
    fprintf(asm_out, "entry_point:\n%s\nbla main ; Call user code.\n%s\n", save_registers_func, restore_registers_func);
    
    std::string line;
    while (getline(asm_src, line)) {
        // Skip this line if it has assembler junk Cemu can't understand
        if (should_skip_line(line.c_str())) {
            continue;
        }

        // Replace the "." in front of ".LC0:" and other GCC-generated labels for local variables.
        // Cemu doesn't like them and tries to interpret as an assembler directive/macro.
        while (line.find(".L") != -1) {
            size_t pos = line.find(".L");
            if (pos != -1) {
                line.replace(pos, 2, "L");
            }
        }
    
        // Replace "$" charcters with an underscore so we don't break the label
        // names for Cemu.
        while (line.find("$") != -1) {
            size_t pos = line.find("$");
            if (pos != -1) {
                line.replace(pos, 1, "_");
            }
        }
    
        // Replace ".asciz" with ".string"
        if (line.find(".asciz") != -1) {
            unsigned long pos = line.find(".asciz");
            line.replace(pos - 1, 8, ".string ");
        }
    
        // Replace "#" comment syntax with ";"
        if (line.find("#") != -1) {
            unsigned long pos = line.find("#");
            line.replace(pos, 1, ";");
        }
    
        // .set macro. Replace it with a new label and a .long.
        // Old:
        // Ltmp0:
        // .set LTOC, Ltmp0+32768

        // New: 
        // Ltmp0:
        // LTOC:
        //     .long Ltmp0+32768
        if (line.find(".set") != -1) {
            line.replace(0, sizeof(".set"), ""); // Delete the .set part
            line.replace(line.find(","), 1, ":\n\t.long");
        }
    
        // I'm sorry for this code...it's really janky, but it does the job.
        // See Appendix B.9 Miscellaneous Mnemonics in this PDF:
        // https://arcb.csc.ncsu.edu/~mueller/cluster/ps3/SDK3.0/docs/arch/PPC_Vers202_Book1_public.pdf
        // la Rx,D(Ry) is equivalent to: addi Rx,Ry,D
        // la Rx,v is equivalent to: addi Rx,Rv,Dv
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
    } // while (getline(asm_src, line))
    
    // Instructions placed after the ".origin" will start replacing from the hooking address.
    fprintf(asm_out, "\n\n.origin = %s", hook_addr); 
    fprintf(asm_out, "\nentry: \nb entry_point ; This branch instruction replaces the original instruction.");
    fprintf(asm_out, "\nreturn_address: ; Branching to this label will return to vanilla code.\n");
    
    fclose(asm_out);
    asm_src.close();
    
}

