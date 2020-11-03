#pragma once

#include "sectionHeaderTable.h"
#include <string>
#include <queue>

using namespace std;

class Arithmetic
{

public:

	static string formAndValidateExpressionString(queue<string> line_of_tokens, string new_symbol, unsigned long lineNo); //remove white char from input expression and check if it is valid
	
	static void parseExpression(string expression_str, queue<Token>& queueOfExpressionTokens);
	
	static uint16_t calculateExpression(queue<Token> queueOfExpressionTokens, bool& succses, string new_symbol, unsigned long lineNo);

};
