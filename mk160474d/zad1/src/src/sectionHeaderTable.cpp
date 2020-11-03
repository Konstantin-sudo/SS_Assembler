#include "sectionHeaderTable.h"

vector<ST_SHT_ENTRY> SectionHeaderTable::table = {
	ST_SHT_ENTRY("", SHT_NULL, 0, 0, 0, 0, 0, 0)
};

void SectionHeaderTable::createEntryAndTable(sht_type table_type, string name)
{
	switch (table_type)
	{
	case SHT_SYMTAB:
	{
		ST_SHT_ENTRY new_entry("symtab", SHT_SYMTAB, 0, 0, 0, 0, 0, sizeof(ST_SYMTAB_ENTRY));
		new_entry.sym_table_reference = new SymbolTable();
		table.push_back(new_entry);
		break;
		//return new_entry.table_reference;
	}
	case SHT_REL:
	{
		ST_SHT_ENTRY new_entry(name, SHT_REL, 0, 0, 0, 0, 0, sizeof(ST_RELTAB_ENTRY));
		new_entry.rel_table_reference = new RelocationTable(name);
		table.push_back(new_entry);
		break;
		//return new_entry.table_reference;
	}
	case SHT_PROGBITS :
	{
		ST_SHT_ENTRY new_entry(name, SHT_PROGBITS, 0, 0, 0, 0, 0, 0);
		new_entry.sec_table_reference = new SectionTable(name);
		table.push_back(new_entry);
		break;
		//return new_entry.table_reference;
	}
	case SHT_NOBITS:
	{
		ST_SHT_ENTRY new_entry(name, SHT_NOBITS, 0, 0, 0, 0, 0, 0);
		new_entry.sec_table_reference = new SectionTable(name);
		table.push_back(new_entry);
		break;
		//return new_entry.table_reference;
	}
	default:
		break;
	}
}

SymbolTable* SectionHeaderTable::getSymbolTable()
{
	for (ST_SHT_ENTRY entry : table)
	{
		if (entry.sh_type == SHT_SYMTAB)
		{
			return entry.sym_table_reference;
		}
	}
	// if not found
	createEntryAndTable(SHT_SYMTAB);
	return table.back().sym_table_reference;
}

RelocationTable* SectionHeaderTable::getRelocationTable(string section_str)
{
	string rel_table_name = ".rel " + section_str;
	for (ST_SHT_ENTRY entry : table)
	{
		if (entry.sh_name == rel_table_name)
		{
			return entry.rel_table_reference;
		}
	}
	//if not found
	createEntryAndTable(SHT_REL, rel_table_name);
	return table.back().rel_table_reference;
}

SectionTable* SectionHeaderTable::getSectionTable(string section_str)
{
	for (ST_SHT_ENTRY entry : table)
	{
		if (entry.sh_name == section_str)
		{
			return entry.sec_table_reference;
		}
	}
	//if not found
	sht_type table_type;
	if (section_str == ".bss")
	{
		table_type = SHT_NOBITS;
	}
	else 
	{
		table_type = SHT_PROGBITS;
	}
	createEntryAndTable(table_type, section_str);
	return table.back().sec_table_reference;
}

void SectionHeaderTable::adjustRelTableForSortedSymTab(vector<ST_RELTAB_ENTRY>& rel_table)
{
	for (auto& entry : rel_table)
	{
		entry.r_info = getSymbolTable()->table[entry.r_info].index_after_sorting;
	}
}

string SectionHeaderTable::int8_to_hex_string(const int8_t b)
{
	stringstream ss;
	uint8_t byte = b;
	ss << hex << ((byte >> 4) & 0xF);
	ss << hex << (byte & 0xF);
	return ss.str();
}

string SectionHeaderTable::uint16_to_hex_string(const uint16_t word)
{
	stringstream ss;
	ss << left << "0x" << hex << word;
	return ss.str();
}

void SectionHeaderTable::writeTablesToOutputTextFile(ofstream& output_file)
{
	const unsigned short width = 20;
	output_file << endl << "//--------------------------------------------SECTION HEADER TABLE--------------------------------------------\\\\" << endl << endl;
	output_file << std::left;
	output_file << setw(width) << "INDEX";
	output_file << setw(width) << "NAME";
	output_file << setw(width) << "TYPE";
	output_file << setw(width) << "ADDRESS";
	//output_file << setw(width) << "OFFSET";
	output_file << setw(width) << "SIZE";
	//output_file << setw(width) << "LINK";
	//output_file << setw(width) << "INFO";
	output_file << setw(width) << "ENTRY SIZE";
	output_file << endl;
	int index = 0;
	for (auto entry : table)
	{
		output_file << setw(width) << to_string(index);
		output_file << setw(width) << entry.sh_name;
		switch (entry.sh_type)
		{
			case sht_type::SHT_NULL:
				output_file << setw(width) << "SHT_NULL";
				break;
			case sht_type::SHT_SYMTAB:
				output_file << setw(width) << "SHT_SYMTAB";
				break;
			case sht_type::SHT_REL:
				output_file << setw(width) << "SHT_REL";
				break;
			case sht_type::SHT_PROGBITS:
				output_file << setw(width) << "SHT_PROGBITS";
				break;
			case sht_type::SHT_NOBITS:
				output_file << setw(width) << "SHT_NOBITS";
				break;
		}
		output_file << setw(width) << uint16_to_hex_string(entry.sh_address);
		//output_file << setw(width) <<  to_string(entry.sh_offset);
		if (entry.sym_table_reference != nullptr) entry.sh_size = entry.sym_table_reference->getSize(); 
		if (entry.rel_table_reference != nullptr) entry.sh_size = entry.rel_table_reference->getSize(); 
		if (entry.sec_table_reference != nullptr) entry.sh_size = entry.sec_table_reference->getSize(); 
		output_file << setw(width) << uint16_to_hex_string(entry.sh_size);
		//output_file << setw(width) << to_string(entry.sh_link);
		//output_file << setw(width) << to_string(entry.sh_info);
		output_file << setw(width) << uint16_to_hex_string(entry.sh_entry_size);
		output_file << endl;
		++index;
	}
	output_file << endl << endl;
	for (auto entry : table) //sym table
	{
		if (entry.sym_table_reference != nullptr) entry.sym_table_reference->writeTableToOutputTextFile(output_file);
	}
	for (auto entry : table) //rel tables
	{
		if (entry.rel_table_reference != nullptr) adjustRelTableForSortedSymTab(entry.rel_table_reference->table);
		if (entry.rel_table_reference != nullptr) entry.rel_table_reference->writeTableToOutputTextFile(output_file);
	}
	for (auto entry : table) //sections
	{
		if (entry.sec_table_reference != nullptr) entry.sec_table_reference->writeTableToOutputTextFile(output_file);
	}
}



void SectionHeaderTable::printTables()
{
	int index = 0;
	cout << endl << endl << "SECTION HEADER TABLE:" << endl;
	cout << "index_table\t" << "name\t" << "type\t" << "address\t" << "offset\t"
		<< "size\t" << "link\t" << "info\t" << "entry_size" << endl;
	for (ST_SHT_ENTRY entry : table)
	{
		cout << index++ << "\t\t"
			<<entry.sh_name << "\t"
			<< entry.sh_type << "\t"
			<< entry.sh_address << "\t"
			<< entry.sh_offset << "\t"
			<< entry.sh_size << "\t"
			<< entry.sh_link << "\t"
			<< entry.sh_info << "\t"
			<< entry.sh_entry_size
			<< endl;
	}
	for (ST_SHT_ENTRY entry : table)
	{
		if (entry.sym_table_reference != nullptr)entry.sym_table_reference->printTable();
		if (entry.rel_table_reference != nullptr)entry.rel_table_reference->printTable();
		if (entry.sec_table_reference != nullptr)entry.sec_table_reference->printTable();
	}


}
