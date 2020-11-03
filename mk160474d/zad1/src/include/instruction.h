#pragma once
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "sectionHeaderTable.h"

#include <map>
#include <queue>
#include <string>

using namespace std;

//#ifndef instruction_type
//#define instruction_type
//#endif // !INSTRUCTION_TYPE


enum addressing_mode
{
	IMMEDIATE = 1,
	REGISTER_DIRECT,
	REGISTER_INDIRECT,
	REGISTER_INDIRECT_WITH_OFFSET,
	MEMORY_DIRECT
};

enum mnemonik_type
{
	TYPE_NULL = 0,
	TYPE_DATA = 1,
	TYPE_JMP = 2
};

struct ST_INSTRUCTION_INFO
{
	int8_t op_code;
	uint16_t number_of_operands;
	mnemonik_type type;

	ST_INSTRUCTION_INFO(int8_t op_code_, uint16_t number_of_operands_, mnemonik_type type_)
	{
		op_code = op_code_;
		number_of_operands = number_of_operands_;
		type = type_;
	};
};

class Instruction
{
public:

	static uint16_t processInstruction(string mnemonik, queue<string> queue_of_operands, string currSection_string, unsigned long LC, unsigned long lineNo); //return number of bytes in instruction_code

private:

	static map<string, ST_INSTRUCTION_INFO> instructions_hash_map;
	static map<addressing_mode, int8_t> addressing_mode_hash_map;

};

#endif