#include "symbolTable.h"

SymbolTable::SymbolTable()
{
	und_section_index = 0;
	ST_SYMTAB_ENTRY new_entry("", 0, LOCAL, und_section_index, false);
	table.push_back(new_entry);
}

void SymbolTable::insertSymbol(Token symbolToken, uint16_t value, string currSection_string, unsigned long lineNo, bool isDefined)
{
	string symbolName = symbolToken.getValue();
	TokenType symbolType = symbolToken.getType();
	
	unsigned long curr_section_index;
	int i = 0;
	for (ST_SYMTAB_ENTRY sym_entry : table)
	{
		if (sym_entry.name == currSection_string)
		{
			curr_section_index = i;
			break;
		}
		++i;
	}

	// if label or symbol(from equ directive)
	if (symbolType == TokenType::LABEL || symbolType==TokenType::SYMBOL)
	{
		//if found
		for (ST_SYMTAB_ENTRY& sym_entry : table)
		{
			if (sym_entry.name == symbolName)
			{
				if (sym_entry.defined)
				{
					throw AsmException("Double definition of symbol: '" + symbolName + "'. Error at line " + to_string(lineNo + 1));
				}
				else
				{
					if (sym_entry.info == symbol_scope::EXTERN)
					{
						throw AsmException("Defining extern symbol: '" + symbolName + "' is not allowed. Error at line " + to_string(lineNo + 1));
					}
					sym_entry.value = value;
					sym_entry.defined = isDefined;
					sym_entry.section_index = curr_section_index;
					if (sym_entry.info != symbol_scope::GLOBAL)
					{
						sym_entry.info = symbol_scope::LOCAL;
					}
					return;
				}
			}
		}
		//if not found
		table.push_back(ST_SYMTAB_ENTRY(symbolName, value, symbol_scope::LOCAL, curr_section_index, isDefined));
	}
	else if (symbolType == TokenType::SECTION_DIRECTIVE)
	{
		i = 0;
		for (ST_SYMTAB_ENTRY& sym_entry : table)
		{
			if (sym_entry.name == symbolName )
			{
				if (sym_entry.defined)
				{
					throw AsmException("Double definition of '" + symbolName + "' section. Error at line " + to_string(lineNo + 1));
				}
				else
				{
					sym_entry.value = value;	// = 0
					sym_entry.section_index = i;
					sym_entry.defined = true;
					return;
				}
			}
			++i;
		}
		//if not found
		curr_section_index = table.size();
		table.push_back(ST_SYMTAB_ENTRY(symbolName, value, symbol_scope::LOCAL, curr_section_index, true));
	}

}

uint16_t SymbolTable::getSymbolValueAndCreateRelTableEntry(string symbol, uint16_t LC, string currSection_string, rel_type relType ) //offset umesto LC
{

	RelocationTable* relocationTable = SectionHeaderTable::getRelocationTable(currSection_string);
	uint16_t ret = 0;
	
	int i = 0;
	bool found = false;
	//if found
	for (ST_SYMTAB_ENTRY& sym_entry : table)
	{
		if (sym_entry.name == symbol)
		{
			found = true;
			if (sym_entry.defined == false && sym_entry.info != symbol_scope::EXTERN)
			{
				ret = 0;
				sym_entry.flink.push_back(ST_FLINK(LC, currSection_string, true, LC, relType));
				break;
			}
			if ( sym_entry.info == symbol_scope::EXTERN) //da li i ekstern ?
			{
				ret = 0;
				relocationTable->createNewEntry(LC, relType, i); //i - sym_entry index in SYM_TAB
				break;
			}
			else
			{
				if (sym_entry.info == symbol_scope::GLOBAL)
				{
					switch (relType)
					{
					case rel_type::R_16:
					{
						ret = 0;
						relocationTable->createNewEntry(LC, relType, i); //i - sym_entry index in SYM_TAB
						break;
					}
					case rel_type::R_16_PC:
					{
						if (sym_entry.section_index == getSectionIndex(currSection_string))
						{
							ret = sym_entry.value;
						}
						else
						{
							ret = 0;
							relocationTable->createNewEntry(LC, relType, i); //i - sym_entry index in SYM_TAB
						}
						break;
					}
					}
					break;
				}
				else // sym_entry.info == symbol_scope::LOCAL 
				{
					switch (relType)
					{
					case rel_type::R_16:
					{
						ret = sym_entry.value;
						relocationTable->createNewEntry(LC, relType, sym_entry.section_index); //i - sym_entry index in SYM_TAB
						break;
					}
					case rel_type::R_16_PC:
					{
						ret = sym_entry.value;
						if (sym_entry.section_index != getSectionIndex(currSection_string))
						{
							relocationTable->createNewEntry(LC, relType, sym_entry.section_index);
						}
						break;
					}
					}
					break;
				}
			}
		}
		++i;
	}

	//if not found
	if (found == false)
	{
		ret = 0;
		ST_FLINK st_flink(LC, currSection_string, true, LC, relType);
		table.push_back(ST_SYMTAB_ENTRY(symbol, 0, symbol_scope::LOCAL, und_section_index, false, st_flink));
		//kako relokacije? --> u backpatching u
	}

	return ret;
}

uint16_t SymbolTable::getSymbolValue(string symbol_name, bool& not_found, unsigned long lineNo)
{
	for (auto entry : table)
	{
		if (entry.name == symbol_name)
		{
			if (entry.defined)
			{
				not_found = false;
				return entry.value;
			}
			else
			{
				if (entry.info == symbol_scope::EXTERN)
				{
					throw AsmException("Extern symbol '" + symbol_name + "' is not allowed as operand in expression at directive .equ. Error at line " + to_string(lineNo + 1));
				}
				not_found = true;
				return 0;
			}
		}
	}
	not_found = true;
	return 0;
}

void SymbolTable::updateSymbolValue(string symbol_name, uint16_t new_value)
{
	for (auto& entry : table)
	{
		if (entry.name == symbol_name)
		{
			entry.value = new_value;
			break;
		}
	}
}

void SymbolTable::setDefined(string symbol_name, bool isDefined)
{
	for (auto& entry : table)
	{
		if (entry.name == symbol_name)
		{
			entry.defined = isDefined;
			break;
		}
	}
}

unsigned long SymbolTable::getIndexOfSectionWhereSymbolIsDefined(string symbol)
{
	for (auto entry : table)
	{
		if (entry.name == symbol)
		{
			return entry.section_index; // if not defined this returns 0;
		}
	}
	return 0; //if not found
}

void SymbolTable::updateAccessRights(string symbol, string access_rights, unsigned long lineNo)
{
	symbol_scope new_scope;
	if (access_rights == ".global")
		new_scope = symbol_scope::GLOBAL;
	else
		new_scope = symbol_scope::EXTERN;
	
	unsigned long i = 0;
	//if found
	for (ST_SYMTAB_ENTRY& sym_entry : table)
	{
		if (sym_entry.name == symbol)
		{
			if (sym_entry.defined == true)
			{
				throw AsmException(access_rights + " directive can't be after definition of symbol '" + sym_entry.name + "'. Error at line " + to_string(lineNo + 1));
			}
			else
			{
				sym_entry.info = new_scope;
				return;
			}
		}
		++i;
	}

	//if not found
	table.push_back(ST_SYMTAB_ENTRY(symbol, 0, new_scope, und_section_index, false));

}

unsigned long SymbolTable::getSectionIndex(string currSection_string)
{
	int i = 0;
	for (auto entry : table)
	{
		if (entry.name == currSection_string)
		{
			return i;
		}
		++i;
	}
	return und_section_index;
}

void SymbolTable::backpatching()
{
	unsigned int symbol_index = 0;
	for (ST_SYMTAB_ENTRY& symbol : table)
	{
		if (symbol.flink.size() != 0)
		{
			if (symbol.defined)
			{
				for (ST_FLINK& flink_entry : symbol.flink)
				{
					switch (symbol.info)
					{
					case symbol_scope::LOCAL:
					{
						if (flink_entry.relocation_unresolved)
						{
							switch (flink_entry.relocation_type)
							{
							case rel_type::R_16:
							{
								SectionHeaderTable::getSectionTable(flink_entry.section)->addWordAtAddress(flink_entry.offset, symbol.value);
								SectionHeaderTable::getRelocationTable(flink_entry.section)->createNewEntry(flink_entry.relocation_offset, flink_entry.relocation_type, symbol.section_index);
								break;
							}
							case rel_type::R_16_PC:
							{
								if (symbol.section_index == getSectionIndex(flink_entry.section))
								{
									SectionHeaderTable::getSectionTable(flink_entry.section)->addWordAtAddress(flink_entry.offset, symbol.value - flink_entry.relocation_offset);
								}
								else
								{
									SectionHeaderTable::getSectionTable(flink_entry.section)->addWordAtAddress(flink_entry.offset, symbol.value);
									SectionHeaderTable::getRelocationTable(flink_entry.section)->createNewEntry(flink_entry.relocation_offset, flink_entry.relocation_type, symbol.section_index);
								}
								break;
							}
							}
						}
						break;
					}
					case symbol_scope::GLOBAL:
					{
						switch (flink_entry.relocation_type)
						{
						case rel_type::R_16:
						{
							if (flink_entry.relocation_unresolved)
							{
								SectionHeaderTable::getRelocationTable(flink_entry.section)->createNewEntry(flink_entry.relocation_offset, flink_entry.relocation_type, symbol_index);
							}
							break;
						}
						case rel_type::R_16_PC:
						{
							if (symbol.section_index == getSectionIndex(flink_entry.section))
							{
								SectionHeaderTable::getSectionTable(flink_entry.section)->addWordAtAddress(flink_entry.offset, symbol.value - flink_entry.relocation_offset); 
							}
							else
							{
								if (flink_entry.relocation_unresolved)
								{
									SectionHeaderTable::getRelocationTable(flink_entry.section)->createNewEntry(flink_entry.relocation_offset, flink_entry.relocation_type, symbol_index);
								}
							}
							break;
						}
						}
						break;
					}
					}
				}
			}
			else // if symbol not defined
			{
				if (symbol.info == symbol_scope::EXTERN)
				{
					for (ST_FLINK& flink_entry : symbol.flink)
					{
						if (flink_entry.relocation_unresolved)
						{
							SectionHeaderTable::getRelocationTable(flink_entry.section)->createNewEntry(flink_entry.relocation_offset, flink_entry.relocation_type, symbol_index);
						}
					}
				}
				else
				{
					throw AsmException("Symbol '" + symbol.name + "' is not defined");
					// ili continue ? - npr za printf da li treba .extern printf
				}
			}
		}
		++symbol_index;
	}
}

void SymbolTable::updateSymbolSize(string symbol, uint16_t new_size)
{
	for (ST_SYMTAB_ENTRY& sym_entry : table)
	{
		if (sym_entry.name == symbol)
		{
			sym_entry.size = new_size;
		}
	}
}

void SymbolTable::sortTable()
{
	sorted_table = vector<ST_SYMTAB_ENTRY>(table.size());
	unsigned long index = 0;
	unsigned long next_section_index = 1;
	unsigned long next_symbol_index = sorted_table.size() - 1;
	for (auto& entry : table)
	{
		if (index != 0)
		{
			entry.index_before_sorting = index;
			if (entry.name.at(0) == '.')
			{
				entry.index_after_sorting = next_section_index;
				sorted_table[next_section_index] = entry;
				++next_section_index;
			}
			else
			{
				entry.index_after_sorting = next_symbol_index;
				sorted_table[next_symbol_index] = entry;
				--next_symbol_index;
			}
		}
		else
		{
			sorted_table[0] = entry;
		}
		++index;
	}
	index = 0;
	for (auto& entry : sorted_table)
	{
		entry.section_index = table[table[entry.index_before_sorting].section_index].index_after_sorting;
	}
}

void SymbolTable::writeTableToOutputTextFile(ofstream& output_file)
{
	sortTable();
	const unsigned short width = 15;
	output_file << endl << "//------------------------------------------------------SYMBOL TABLE------------------------------------------------------\\\\" << endl << endl;
	output_file << std::left;
	output_file << setw(width) << "INDEX";
	output_file << setw(width) << "NAME";
	output_file << setw(width) << "VALUE";
	output_file << setw(width) << "SECTION";
	output_file << setw(width) << "INFO";
	output_file << setw(width) << "DEFINED";
	output_file << setw(width) << "SIZE";
	output_file << setw(width) << "FLINK(Offset/Section)";
	output_file << endl;

	int index = 0;
	for (ST_SYMTAB_ENTRY entry : sorted_table)
	{
		output_file << setw(width) << to_string(index);
		if (index == 0)
			output_file << setw(width) << "UND";
		else
			output_file << setw(width) << entry.name;
		if(entry.defined)
			output_file << setw(width) << SectionHeaderTable::uint16_to_hex_string(entry.value);
		else
			output_file << setw(width) << "n/a";
		output_file << setw(width) << to_string(entry.section_index);
		switch (entry.info)
		{
		case symbol_scope::LOCAL:
			output_file << setw(width) << "local";
			break;
		case symbol_scope::GLOBAL:
			output_file << setw(width) << "global";
			break;
		case symbol_scope::EXTERN:
			output_file << setw(width) << "extern";
			break;
		}
		if (entry.defined)
			output_file << setw(width) << "true";
		else
			output_file << setw(width) << "false";
		if(entry.defined && entry.name.at(0) == '.')
			output_file << setw(width) << SectionHeaderTable::uint16_to_hex_string(entry.size);
		else
			output_file << setw(width) << "n/a";
		if (entry.flink.size() == 0)
		{
			output_file << setw(width) << "null";
		}
		else
		{
			for (ST_FLINK st : entry.flink)
				output_file << setw(width) << SectionHeaderTable::uint16_to_hex_string(st.offset) + "/" + st.section;
		}
		
		output_file << endl;
		++index;
	}
	output_file << endl << endl;
}

uint16_t SymbolTable::getSize()
{
	return static_cast<uint16_t>(table.size() * sizeof(ST_SYMTAB_ENTRY));
}

void SymbolTable::printTable()
{
	int index = 0;
	cout << endl<<"SYMBOL TABLE:"<<endl;
	cout << "index_table\t" << "name\t" << "value\t" << "section_index\t" << "info\t" << "defined\t" << "size\t" << "flink(offset)\t" << endl;
	
	for (ST_SYMTAB_ENTRY entry : table)
	{
		cout << index++ << "\t\t"
			<< entry.name << "\t"
			<< entry.value << "\t"
			<< entry.section_index << "\t\t"
			<< entry.info << "\t"
			<< entry.defined << "\t"
			<< entry.size;
		for (ST_FLINK st : entry.flink)
			cout << st.offset << " ";
		cout << endl;
	}
}

