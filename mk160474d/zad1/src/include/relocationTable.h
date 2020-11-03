#pragma once
#ifndef RELOCATIONTABLE_H
#define RELOCATIONTABLE_H

#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

enum rel_type
{
	R_16,		//apsolut
	R_16_PC		//pc relative
};

struct ST_RELTAB_ENTRY
{
	uint16_t r_offset;		//offset from start of the secion
	rel_type r_type;		//rel tpe
	unsigned long r_info;	//index of symbol from sym_table whose value linker should insert

	//unsigned long relevant_symbol_index;
	ST_RELTAB_ENTRY(uint16_t r_offset_, rel_type r_type_, unsigned long r_info_)
	{
		r_offset = r_offset_;
		r_type = r_type_;
		r_info = r_info_;
		//relevant_symbol_index = relevant_symbol_index_;
	}

};

struct ST_SYMTAB_ENTRY;

class RelocationTable
{
public:

	friend class SectionHeaderTable;

	static string uint16_to_hex_string(const uint16_t number);

	RelocationTable(string name_);

	void createNewEntry(uint16_t r_offset, rel_type r_type, unsigned long r_info);
	
	void writeTableToOutputTextFile(ofstream& output_file);

	uint16_t getSize();

	void printTable();

private:
	string name;
	vector<ST_RELTAB_ENTRY> table;

};

#endif