#include "token.h"
#include <iostream>

Token Token::parseToken(string token)
{
	TokenType type;
	//iterate through syntax and find a match
	for (unsigned long i = 0; i < SYNTAX_SIZE; ++i )
	{
		if (regex_match(token, assemblerSyntaxRegex[i]))
		{
			//std::cout << "\n\n"<<i<<".MATCH " + token;
			switch (i)
			{
				case 0:
				{
					type = TokenType::LABEL;
					return Token(token.substr(0, token.size() - 1), type);
				}
				case 1:
				{
					type = TokenType::INSTRUCTION;
					return Token(token, type);
				}
				case 2:
				{
					type = TokenType::ACCESS_RIGHTS_DIRECTIVE;
					return Token(token, type);
				}
				case 3:
				{
					type = TokenType::MEMORY_ALLOCATION_DIRECTIVE;
					return Token(token, type);
				}
				case 4:
				{
					type = TokenType::EQU_DIRECTIVE;
					return Token(token, type);
				}
				case 5:
				{
					type = TokenType::END_DIRECTIVE;
					return Token(token, type);
				}
				case 6:
				{
					type = TokenType::SECTION_DIRECTIVE;
					return Token(token, type);
				}
				case 7:
				{
					type = TokenType::OPERAND_IMMEDIATE_HEX_DATA;
					return Token(token, type);
				}
				case 8:
				{
					type = TokenType::OPERAND_IMMEDIATE_DEC_DATA;
					return Token(token, type);
				}
				case 9:
				{
					type = TokenType::OPERAND_IMMEDIATE_SYMBOL_DATA;
					return Token(token, type);
				}
				case 10:
				{
					token = regex_replace(token, regex("sp"), "r6");
					token = regex_replace(token, regex("pc"), "r7");
					token = regex_replace(token, regex("psw"), "r15");
					type = TokenType::OPERAND_REGISTER_DIRECT_ADDRESSING_DATA; 
					return Token(token, type);
				}
				case 11:
				{
					token = regex_replace(token, regex("sp"), "r6");
					token = regex_replace(token, regex("pc"), "r7");
					token = regex_replace(token, regex("psw"), "r15");
					type = TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_DATA;
					return Token(token, type);
				}
				case 12:
				{
					token = regex_replace(token, regex("sp"), "r6");
					token = regex_replace(token, regex("pc"), "r7");
					token = regex_replace(token, regex("psw"), "r15");
					type = TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_WITH_OFFSET_DATA;
					return Token(token, type);
				}
				case 13:
				{
					type = TokenType::HEX_NUMBER;
					return Token(token, type);
				}
				case 14:
				{
					type = TokenType::DEC_NUMBER;
					return Token(token, type);
				}
				case 15:
				{
					type = TokenType::SYMBOL;
					return Token(token, type);
				}
				case 16:
				{
					token = regex_replace(token, regex("sp"), "r6");
					token = regex_replace(token, regex("pc"), "r7");
					token = regex_replace(token, regex("psw"), "r15");
					type = TokenType::OPERAND_REGISTER_DIRECT_ADDRESSING_JMP;
					return Token(token, type);
				}
				case 17:
				{
					token = regex_replace(token, regex("sp"), "r6");
					token = regex_replace(token, regex("pc"), "r7");
					token = regex_replace(token, regex("psw"), "r15");
					type = TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_JMP;
					return Token(token, type);
				}
				case 18:
				{
					token = regex_replace(token, regex("sp"), "r6");
					token = regex_replace(token, regex("pc"), "r7");
					token = regex_replace(token, regex("psw"), "r15");
					type = TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_WITH_OFFSET_JMP;
					return Token(token, type);
				}
				case 19:
				{
					type = TokenType::OPERAND_ABSOLUTE_HEX_JMP;
					return Token(token, type);
				}
				case 20:
				{
					type = TokenType::OPERAND_ABSOLUTE_DEC_JMP;
					return Token(token, type);
				}
				case 21:
				{
					type = TokenType::OPERAND_ABSOLUTE_SYMBOL_JMP;
					return Token(token, type);
				}
				case 22:
				{
					type = TokenType::EXPRESSION;
					return Token(token, type);
				}
				case 23:
				{
					type = TokenType::OPERATOR;
					return Token(token, type);
				}
				default:
					break;
			}
		}
		else
		{
			//std::cout << "\n\n"<<i<<".SEARCH " + token;
		}
	}//end for

	return Token(token, TokenType::INVALID);
}
