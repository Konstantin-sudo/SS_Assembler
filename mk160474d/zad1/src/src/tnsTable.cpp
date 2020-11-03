#include "tnsTable.h"

vector<ST_TNSTAB_ENTRY> TNS_Table::table;

void TNS_Table::insert(string symbol_name, queue<Token> expression)
{
	table.push_back(ST_TNSTAB_ENTRY(symbol_name, expression));
}

void TNS_Table::resolveUndefinedSymbols()
{
	unsigned long symbols_uneresolved_cnt = table.size();
	while (symbols_uneresolved_cnt > 0)
	{
		unsigned long resolved_cnt = 0;
		for (auto& entry : table)
		{
			if (entry.resolved == false)
			{
				bool succses = false;
				uint16_t value = Arithmetic::calculateExpression(entry.queueOfArithmeticTokens, succses, entry.symbol_name, -1);
				if (succses)
				{
					SectionHeaderTable::getSymbolTable()->setDefined(entry.symbol_name);
					SectionHeaderTable::getSymbolTable()->updateSymbolValue(entry.symbol_name, value);
					entry.resolved = true;
					++resolved_cnt;
					--symbols_uneresolved_cnt;
				}
			}
		}
		if (resolved_cnt == 0)
		{
			throw AsmException("ERROR: Unable to resolve all symbols from equ directives. Possible infinite loop created");
		}

	}

}

void TNS_Table::writeTableToOutputTextFile(ofstream& output_file)
{
	const unsigned short width = 20;
	output_file << endl << "//----------TNS TABLE----------\\\\" << endl << endl;
	output_file << std::left;
	output_file << setw(width) << "SYMBOL NAME";
	output_file << setw(width) << "EXPRESSION";
	output_file << endl;

	for (auto entry : table)
	{
		output_file << setw(width) << entry.symbol_name;
		queue<Token> q = entry.queueOfArithmeticTokens;
		int n = q.size();
		for (int i = 0; i < n; ++i)
		{
			Token t = q.front();
			q.pop();
			output_file << t.getValue();
		}
		output_file << endl;
	}
	output_file << endl;
}

void TNS_Table::printTable()
{
	cout << endl << "TNS TABLE:" << endl;
	cout << "Symbol name\t" << "Expression\t" << endl;

	for (auto entry : table)
	{
		cout << entry.symbol_name << "\t\t";

		queue<Token> q = entry.queueOfArithmeticTokens;
		int n = q.size();
		for (int i = 0; i < n; ++i)
		{
			Token t = q.front();
			q.pop();
			cout << t.getValue();
		}
		cout << endl;
	}
}
