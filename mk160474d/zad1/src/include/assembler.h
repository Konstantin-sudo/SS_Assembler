#pragma once
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "tnsTable.h"
#include "instruction.h"
#include <vector>
#include <queue>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

enum SectionType {
	NO_SECTION = 0,
	BSS = 1,
	DATA = 2,
	TEXT = 3,
	RODATA = 4,
	USER_SECTION = 5
};

class Assembler{

	public:
	
		static int generateObjectProgramInTextFormat(string input_file_name, string output_file_name);
		
	private:
		Assembler() {};
		~Assembler() {};
		
		static void readAndTokenizeInputFile(string input_file_name);
		static void removeDelimitersAndCommentsFromLine(const string& line, queue<string>& tokenizedLine, const string& delimiters);
		static void firstPass();
		static void backpatching();
		static void writeToOutputTextFile(string output_file_name);

	//fields
		static vector<queue<string>> tokenizedInputFile;

};
#endif
