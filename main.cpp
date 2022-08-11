#include <iostream>
#include <string>
#include <fstream>
#include <conio.h>


int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        std::cout << "Select a C or C++ source file.\n\nPress any key to exit...\n";
        char _dummy = _getch();
        return 1;
    }
    else
    {
        // Get file extension
        std::string temp(argv[1]);
        size_t i = temp.rfind('.', temp.length());
        std::string FileExtension = temp.substr(i + 1, temp.length() - i);

        std::string command = "C:\\devkitPro\\devkitPPC\\bin\\powerpc-eabi-g++.exe -emit-llvm -O1 -mbig-endian -m32 -fno-ident ";
        if (FileExtension == "cpp")
        {
            command.append("-std=c++11 -fno-rtti ");
        }
        command.append("-fno-toplevel-reorder -mregnames -fno-function-sections -ffreestanding -fno-data-sections -msdata=none ");
        command.append("-mno-sdata -fno-exceptions -fno-omit-frame-pointer -fno-asynchronous-unwind-tables -falign-functions=4 ");
        command.append("-falign-labels=4 -falign-jumps=4 -falign-loops=4 -S -otemp.asm ");
        command.append(argv[1]);
        system(const_cast<char*>(command.c_str())); // Run G++ compile
        std::cout << command;

        std::fstream AssemblySrc;
        std::ofstream AssemblyOut;
        AssemblySrc.open("temp.asm", std::ios::in);
        if (AssemblySrc.is_open())
        {
            std::string line;
            AssemblyOut.open("output.asm", std::ios::out);
            while (getline(AssemblySrc, line))
            {
                // Remove compiler clutter
                if (line.find("	.file") == 0 || line.find("	.text") == 0 || line.find("	.globl") == 0 ||
                    line.find("	.type") == 0 || line.find("	.size") == 0 || line.find("	.section") == 0 ||
                    line.find("	.weak") == 0 || line.find("	.data") == 0 || line.find("	.ident") == 0 ||
                    line.find("	.addrsig") == 0 || line.find("	.addrsig_sym") == 0 || line.find("	.machine") == 0 ||
                    line.find("	.align") == 0 || line.find("	.gnu_attribute") == 0 || line.find("	.lcomm") == 0)
                {
                    continue; // Skip this line
                }

                // Replace GCC's weird "%r#" register notation
                while (line.find("%r") != -1)
                {
                    size_t pos = line.find("%r");
                    if (pos != -1)
                    {
                        line.replace(pos, 2, "r");
                    }
                }
                while (line.find("%cr") != -1)
                {
                    size_t pos = line.find("%cr");
                    if (pos != -1)
                    {
                        line.replace(pos, 3, "cr");
                    }
                }

                // Print to file
                AssemblyOut << line << "\n";
            }
            AssemblyOut.close();
            AssemblySrc.close();

            remove("temp.asm");
        }
    }
}
