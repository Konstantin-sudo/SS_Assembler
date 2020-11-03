#pragma once

#include "arithmetic.h"
#include <string>
#include <vector>
#include <queue>
using namespace std;

struct ST_TNSTAB_ENTRY
{
	string symbol_name;
	queue<Token> queueOfArithmeticTokens; //expression
	bool resolved;

	ST_TNSTAB_ENTRY() {};
	ST_TNSTAB_ENTRY(string sym_name, queue<Token> expression)
	{
		symbol_name = sym_name;
		queueOfArithmeticTokens = expression;
		resolved = false;
	};
};


class TNS_Table
{
public:

	static void insert(string symbol_name, queue<Token> expression);
	
	static void resolveUndefinedSymbols();

	static void writeTableToOutputTextFile(ofstream& output_file);

	static void printTable();

private:

	static vector<ST_TNSTAB_ENTRY> table;

};