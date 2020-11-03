#pragma once
#ifndef SECTIONHEADERTABLE_H
#define SECTIONHEADERTABLE_H

#include "symbolTable.h"
#include "sectionTable.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

class SymbolTable;
class RelocationTable;

enum sht_type
{
	SHT_NULL,		//null
	SHT_SYMTAB,		//symbol table
	SHT_REL,		//rel table
	SHT_PROGBITS,	//sections with content - .text, .data, .rodata ?
	SHT_NOBITS,		//sectoins without content - .bss	
	//SHT_STRTAB	//string table
};

struct ST_SHT_ENTRY
{
	string sh_name;			//ex: '.rel.text', '.text' ,'symtab', ...  or address in section of all strings  .
	sht_type sh_type;			//.text/bss/data/rodata/symtable/rel
	//flags;
	uint16_t sh_address;		// = 0 always
	uint16_t sh_offset;			//offset from the begining of the output file -- where this section starts
	uint16_t sh_size;			// size in bytes
	uint16_t sh_link;			//index of sym_table in SHT (99% there will be one sym_table == one same index everywhere)
	uint16_t sh_info;			//ex: type == sht_rel, then info tells if it is text,bss,data,rodata -- type:rel,info:text-->.rel.text table
	//addr_align;
	uint16_t sh_entry_size;	//size of one entry in bytes

	SymbolTable* sym_table_reference;
	RelocationTable* rel_table_reference;
	SectionTable* sec_table_reference;

	ST_SHT_ENTRY(string name_, sht_type type_, uint16_t address_, uint16_t offset_, uint16_t size_, uint16_t link_, uint16_t info_, uint16_t entry_size_)
	{
		sh_name = name_; 
		sh_type = type_;
		sh_address = address_;
		sh_offset = offset_;
		sh_size = size_;
		sh_link = link_;
		sh_info = info_;
		sh_entry_size = entry_size_;

		sym_table_reference = nullptr;
		rel_table_reference = nullptr;
		sec_table_reference = nullptr;
	}
};

class SectionHeaderTable
{
public:

	static void createEntryAndTable(sht_type table_type, string name = "");

	static SymbolTable* getSymbolTable();

	static RelocationTable* getRelocationTable(string section_str);
	
	static SectionTable* getSectionTable(string section_str);
	
	static void writeTablesToOutputTextFile(ofstream& output_file);

	static void adjustRelTableForSortedSymTab(vector<ST_RELTAB_ENTRY>& rel_table);

	static string int8_to_hex_string(const int8_t b);

	static string uint16_to_hex_string(const uint16_t number);

	static void printTables();

private:
	static vector<ST_SHT_ENTRY> table;
};

#endif