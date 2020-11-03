#pragma once
#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "token.h"
#include "relocationTable.h"
#include "sectionHeaderTable.h"

#include <string>
#include <iostream>

using namespace std;


enum symbol_scope
{
	LOCAL = 1,
	GLOBAL,
	EXTERN
};

struct ST_FLINK
{
	uint16_t offset;	//offset from start of the section where new defined symbol value should be inserted
	string section;

	bool relocation_unresolved;
	uint16_t relocation_offset;
	rel_type relocation_type;
	
	ST_FLINK() { section = ""; };

	ST_FLINK(uint16_t offset_, string section_, bool relocation_unresolved_ = false, uint16_t relocation_offset_ = 0, rel_type relocation_type_ = rel_type::R_16)
	{
		offset = offset_;
		section = section_;

		relocation_unresolved = relocation_unresolved_;
		relocation_offset = relocation_offset_;
		relocation_type = relocation_type_;
	}
};

struct ST_SYMTAB_ENTRY
{
	string name;
	uint16_t value;
	symbol_scope info;
	unsigned long section_index;
	uint16_t size; //only for section symbols
	bool defined;
	vector<ST_FLINK> flink;
	//unsigned long index;
	unsigned long index_before_sorting; //for sorting table
	unsigned long index_after_sorting; //for sorting table

	ST_SYMTAB_ENTRY() {}

	ST_SYMTAB_ENTRY(string name_, uint16_t value_, symbol_scope info_, unsigned long section_index_, bool defined_ /*,unsigned long index_*/, ST_FLINK st_flink_ = ST_FLINK())
	{
		name = name_;
		value = value_;
		info = info_;
		section_index = section_index_;
		size = 0;
		defined = defined_;
		//index = index_;
		if (st_flink_.section != "") 
		{
			flink.push_back(st_flink_);
		}
		index_before_sorting = 0;
		index_after_sorting = 0;
	}
};


class SymbolTable
{
public:

	friend class SectionHeaderTable;

	SymbolTable();

	void insertSymbol(Token symbolToken, uint16_t value, string currSection_string, unsigned long lineNo, bool isDefined = true);
	
	uint16_t getSymbolValueAndCreateRelTableEntry(string symbol, uint16_t LC, string currSection_string,  rel_type relType);
	
	uint16_t getSymbolValue(string symbol_name, bool& not_found, unsigned long lineNo);

	void updateSymbolValue(string symbol_name, uint16_t new_value);

	void setDefined(string symbol_name, bool isDefined = true);

	unsigned long getIndexOfSectionWhereSymbolIsDefined(string symbol);

	void updateAccessRights(string symbol, string access_rights, unsigned long lineNo);
	
	unsigned long getSectionIndex(string currSection_string);
	
	void backpatching();

	void updateSymbolSize(string simbol, uint16_t new_size);

	void sortTable(); // sorting table - section first, then others symbols
	
	void writeTableToOutputTextFile(ofstream& output_file);
	
	uint16_t getSize();

	void  printTable();

private:

	unsigned long und_section_index;
	
	vector<ST_SYMTAB_ENTRY> table;	//name->entry
	vector<ST_SYMTAB_ENTRY> sorted_table;	//name->entry

};

#endif

