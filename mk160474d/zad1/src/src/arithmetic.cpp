#include "arithmetic.h"

string Arithmetic::formAndValidateExpressionString(queue<string> expression_tokens, string new_symbol, unsigned long lineNo)
{
	string expression = "";
	bool expect_operator = false;
	int number_of_expressions_in_expression = expression_tokens.size();
	for (int i = 0; i < number_of_expressions_in_expression; ++i)
	{
		Token currToken = Token::parseToken(expression_tokens.front());
		expression_tokens.pop();
		switch (currToken.getType())
		{
		case TokenType::OPERATOR:
		{
			if (expect_operator == false && expression != "")
			{
				throw AsmException("Invalid syntax in expression in '.equ " + new_symbol + "' directive. Error at line: " + to_string(lineNo + 1));
			}
			expect_operator = false;
			expression += currToken.getValue();
			break;
		}
		case TokenType::DEC_NUMBER:
		{
			long value = stol(currToken.getValue(), NULL, 0);;
			if (value > 32767 || value < -32767)
			{
				throw AsmException("Number '" + currToken.getValue() + "' at directive '.equ " + new_symbol + "' must be in range [-32767,32767]. Error at line " + to_string(lineNo + 1));
			}
		}
		case TokenType::EXPRESSION:
		{
			//+123 / -123 / 123
			if (currToken.getValue().at(0) == '+' || currToken.getValue().at(0) == '-')
			{
				if (expect_operator == false && expression != "")
				{
					throw AsmException("Invalid syntax in expression in '.equ " + new_symbol + "' directive. Error at line: " + to_string(lineNo + 1));
				}
			}
			else if (expect_operator)
			{
				throw AsmException("Invalid syntax in expression in '.equ " + new_symbol + "' directive. Error at line: " + to_string(lineNo + 1));
			}
			if (currToken.getValue().at(currToken.getValue().size() - 1) == '+' || currToken.getValue().at(currToken.getValue().size() - 1) == '-')
			{
				expect_operator = false;
			}
			else
			{
				expect_operator = true;
			}
			expression += currToken.getValue();
			break;
		}
		case TokenType::HEX_NUMBER:
		{
			long value = stol(currToken.getValue(), NULL, 0);;
			if (value > 65535)
			{
				throw AsmException("Number '" + currToken.getValue() + "' at directive '.equ " + new_symbol + "' must be in range [0x0000, 0xFFFF]. Error at line " + to_string(lineNo + 1));
			}
		}
		case TokenType::SYMBOL:
		case TokenType::SECTION_DIRECTIVE:
		{
			if (currToken.getValue() == ".section")
			{
				throw AsmException("Invalid operand '" + currToken.getValue() + "' in expression at directive '.equ " + new_symbol + "'. Error at line " + to_string(lineNo + 1));
			}
			if (expect_operator)
			{
				throw AsmException("Invalid syntax in expression in '.equ " + new_symbol + "' directive. Error at line: " + to_string(lineNo + 1));
			}
			expect_operator = true;
			expression += currToken.getValue();
			break;
		}
		default:
			throw AsmException("Invalid syntax in expression in '.equ " + new_symbol + "' directive. Error at line: " + to_string(lineNo + 1));
			break;
		}
	}
	if (expression.at(expression.size() - 1) == '+' || expression.at(expression.size() - 1) == '-')
	{
		throw AsmException("Expression at directive '.equ " + new_symbol + ", " + expression + "' can't end with operator. Error at line: " + to_string(lineNo + 1));
	}
    return expression;
}

void Arithmetic::parseExpression(string expression_str, queue<Token>& queueOfExpressionTokens)
{
	unsigned long i = 0;
	while( i < expression_str.size())
	{
		char c = expression_str[i];
		if (c == '+' || c == '-')
		{
			string operator_;
			operator_ += c;
			queueOfExpressionTokens.push(Token::parseToken(operator_));
			++i;
		}
		else
		{
			size_t position_of_next_operator = expression_str.find_first_of("+-", i);
			string operand = expression_str.substr(i, position_of_next_operator - i);
			queueOfExpressionTokens.push(Token::parseToken(operand));
			i = position_of_next_operator;
		}
	}

}

uint16_t Arithmetic::calculateExpression(queue<Token> queueOfExpressionTokens, bool& succses, string new_symbol, unsigned long lineNo)
{
	uint16_t result = 0;
	bool add = true;
	int number_of_tokens_in_expression = queueOfExpressionTokens.size();
	for (int i = 0; i < number_of_tokens_in_expression; ++i)
	{
		Token current_token = queueOfExpressionTokens.front();
		queueOfExpressionTokens.pop();
		if (current_token.getType() == TokenType::OPERATOR)
		{
			if (current_token.getValue() == "+")
			{
				add = true;
			}
			else // == "-"
			{
				add = false;
			}
		}
		else if (current_token.getType() == TokenType::DEC_NUMBER || current_token.getType() == TokenType::HEX_NUMBER)
		{
			long number = stol(current_token.getValue(), NULL, 0);
			if ((number > 32767 || number < -32767) && current_token.getType() == TokenType::DEC_NUMBER)
			{
				throw AsmException("Number '" + current_token.getValue() + "' in expression at directive '.equ " + new_symbol + "' must be in range [-32767,32767]. Error at line " + to_string(lineNo + 1));
			}
			if (number > 65535 && current_token.getType() == TokenType::HEX_NUMBER)
			{
				throw AsmException("Number '" + current_token.getValue() + "' in expression at directive '.equ " + new_symbol + "' must be in range [0x0000, 0xFFFF]. Error at line " + to_string(lineNo + 1));
			}
			if (add)
			{
				result += static_cast<uint16_t>(number);
			}
			else
			{
				result -= static_cast<uint16_t>(number);
			}
		}
		else if (current_token.getType() == TokenType::SYMBOL || current_token.getType() == TokenType::SECTION_DIRECTIVE)
		{
			if (current_token.getValue() == ".section")
			{
				throw AsmException("Invalid operand '" + current_token.getValue() + "' in expression at directive '.equ " + new_symbol + "'. Error at line " + to_string(lineNo + 1));
			}
			bool symbol_not_found = true;
			uint16_t symbol_value = SectionHeaderTable::getSymbolTable()->getSymbolValue(current_token.getValue(), symbol_not_found, lineNo);
			if (symbol_not_found)
			{
				succses = false;
				return 0;
			}
			else
			{
				if (add)
				{
					result += symbol_value;
				}
				else
				{
					result -= symbol_value;
				}
			}
		}
		else
		{
			throw AsmException("Invalid operand '" + current_token.getValue() + "' in expression at directive '.equ " + new_symbol + "'. Error at line " + to_string(lineNo + 1));
		}
	}
	succses = true;
	return result;
}

