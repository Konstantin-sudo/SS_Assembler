#pragma once
#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <regex>
#include "asmexception.h"
using namespace std;

const unsigned long SYNTAX_SIZE = 24;

//assemblerSyntaxRegex and TokenType order must match!!!
static const regex assemblerSyntaxRegex[SYNTAX_SIZE] = {
	//label
	regex("^([a-zA-Z_][a-zA-Z0-9_]*_{0,}):$"),		//label - must start with character a-z/A-Z/_
	//instruction
	regex("^(halt|iret|ret|int|call|jmp|jeq|jne|jgt|((push|pop|xchg|mov|add|sub|mul|div|cmp|not|and|or|xor|test|shl|shr)(b|w){0,1}))$"),	//instruction
	//directives
	regex("^\\.(global|extern)$"),					//access rights directive
	regex("^\\.(byte|word|skip)$"),					//memmory allocation directive
	regex("^\\.equ$"),								//equ directive		
	regex("^\\.end$"),								//end directive	
	regex("^\\.(section|text|bss|data|rodata|([a-zA-Z_][a-zA-Z0-9_]*_{0,}))$"),		//section directive
	//data operand
	regex("^\\$0x[0-9a-f]{1,}$"),					//operand immediate hex_data
	regex("^\\$(\\-|\\+){0,1}[0-9]{1,}$"),			//operand immediate dec_data
	regex("^\\$([a-zA-Z_][a-zA-Z0-9_]*_{0,})$"),	//operand_immediate symbol_data
	regex("^%((r[0-7])|psw|pc|sp)(h|l){0,1}$"),		//operand register direct addressing_data
	regex("^\\(%((r[0-7])|psw|pc|sp)\\)$"),			//operand register indirect addressing_data
	regex("^(\\-|\\+){0,1}[a-zA-z0-9_]{1,}\\(%((r[0-7])|psw|pc|sp)\\)$"),//operand reqister indirect with 16-bit offset_data
	//data & jmp operand
	regex("^0x[0-9a-f]{1,}$"),										//hex number / operand absolute hex_data || operand immediate hex_jmp
	regex("^(\\-|\\+){0,1}[0-9]{1,}$"),								//dec number / operand absolute dec_data || operand immediate dec_jmp
	regex("^([a-zA-Z_][a-zA-Z0-9_]*_{0,})$"),						//symbol / operand absolute symbol_data || operand_immediate_symbol_jmp
	//jmp operand
	regex("^\\*%((r[0-7])|psw|pc|sp)(h|l){0,1}$"),					//operand register direct addressing_jmp
	regex("^\\*\\(%((r[0-7])|psw|pc|sp)\\)$"),						//operand register indirect addressing_jmp
	regex("^\\*(\\-|\\+){0,1}[a-zA-z0-9_]{1,}\\(%((r[0-7])|psw|pc|sp)\\)$"),//operand reqister indirect with 16-bit offset_jmp
	regex("^\\*0x[0-9a-f]{1,}$"),									//operand absolute hex_jmp
	regex("^\\*(\\-|\\+){0,1}[0-9]{1,}$"),							//operand absolute dec_jmp
	regex("^\\*([a-zA-Z_][a-zA-Z0-9_]*_{0,})$"),					//operand absolute symbol_jmp
	//expressions
	regex("^(\\+|\\-){0,1}(([a-zA-Z_][a-zA-Z0-9_]*_{0,})|([0-9]{1,})|(0x[0-9a-f]{1,})|(\\.(section|text|bss|data|rodata|([a-zA-Z_][a-zA-Z0-9_]*_{0,}))))((\\+|\\-)(([a-zA-Z_][a-zA-Z0-9_]*_{0,})|([0-9]{1,})|(0x[0-9a-f]{1,})|(\\.(section|text|bss|data|rodata|([a-zA-Z_][a-zA-Z0-9_]*_{0,})))))*(\\+|\\-){0,1}$"), //expresion
	regex("^(\\+|\\-)$"), //operator
};

enum TokenType 
{
	INVALID = -1,
	LABEL = 0,										//label1:,...
	INSTRUCTION = 1,								//mov,jmp,...
	
	ACCESS_RIGHTS_DIRECTIVE = 2,					//.global, .extern
	MEMORY_ALLOCATION_DIRECTIVE = 3,				//.byte, .word, .skip
	EQU_DIRECTIVE = 4,								//.equ
	END_DIRECTIVE =	5,								//.end
	SECTION_DIRECTIVE = 6,							//.section, .text, .bss, .data, .rodata,...

	OPERAND_IMMEDIATE_HEX_DATA,
	OPERAND_IMMEDIATE_DEC_DATA,
	OPERAND_IMMEDIATE_SYMBOL_DATA,
	OPERAND_REGISTER_DIRECT_ADDRESSING_DATA,
	OPERAND_REGISTER_INDIRECT_ADDRESSING_DATA,
	OPERAND_REGISTER_INDIRECT_ADDRESSING_WITH_OFFSET_DATA,

	HEX_NUMBER,		// == OPERAND_ABSOLUTE_HEX_DATA__OPERAND_IMMEDIATE_HEX_JMP,
	DEC_NUMBER,		// == OPERAND_ABSOLUTE_DEC_DATA__OPERAND_IMMEDIATE_DEC_JMP
	SYMBOL,			// == OPERAND_ABSOLUTE_SYMBOL_DATA__OPERAND_IMMEDIATE_SYMBOL_JMP

	OPERAND_REGISTER_DIRECT_ADDRESSING_JMP,
	OPERAND_REGISTER_INDIRECT_ADDRESSING_JMP,
	OPERAND_REGISTER_INDIRECT_ADDRESSING_WITH_OFFSET_JMP,
	OPERAND_ABSOLUTE_HEX_JMP,
	OPERAND_ABSOLUTE_DEC_JMP,
	OPERAND_ABSOLUTE_SYMBOL_JMP,
	
	EXPRESSION,
	OPERATOR
};


class Token 
{
	public:

		Token() :value(""), type(INVALID) {}
		
		Token(string value_, TokenType type_) :value(value_), type(type_) {}

		string getValue() { return value; }
		
		TokenType getType() { return type; }
	

		static Token parseToken(string token);

private:
		string value;
		TokenType type;
};

#endif