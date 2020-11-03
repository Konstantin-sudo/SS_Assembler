// Sistemski_Softver_Projekat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "assembler.h"

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::cout << "Invalid number of command line arguments. Syntax should be: 'assembler -o output_file_name.o input_file_name.s'  \n";
        return -1;
    }
    if (string(argv[1]) != "-o")
    {
        std::cout << "Invalid syntax at command line arguments (2). Syntax should be: 'assembler -o output_file_name.o input_file_name.s'  \n";
        return -1;
    }

    int ret = -1;
    try 
    {
        ret = Assembler::generateObjectProgramInTextFormat(argv[3], argv[2]);
        cout << "Assembler finished succesfully" << endl;
    }
    catch (AsmException e)
    {
        cout << endl << e.getMsg() << endl;
    }
    catch (...)
    {
        cout << endl << "Unexpected error" << endl;
    }
    return ret;
}


