#include "relocationTable.h"

RelocationTable::RelocationTable(string name_)
{
	name = name_;
}

void RelocationTable::createNewEntry(uint16_t r_offset, rel_type r_type, unsigned long r_info)
{
	table.push_back(ST_RELTAB_ENTRY(r_offset, r_type, r_info));
}

string RelocationTable::uint16_to_hex_string(const uint16_t word)
{
	stringstream ss;
	ss << left << "0x" << hex << word;
	return ss.str();
}

void RelocationTable::writeTableToOutputTextFile(ofstream& output_file)
{
	const unsigned short width = 20;
	output_file << endl << "//---------RELOCATION TABLE " + name + "---------\\\\" << endl << endl;
	output_file << std::left;
	output_file << setw(width) << "R_OFFSET";
	output_file << setw(width) << "R_TYPE";
	output_file << setw(width) << "R_INFO";
	output_file << endl;

	for (ST_RELTAB_ENTRY entry : table)
	{
		output_file << setw(width) << uint16_to_hex_string(entry.r_offset);
		switch (entry.r_type)
		{
		case rel_type::R_16:
			output_file << setw(width) << "R_16";
			break;
		case rel_type::R_16_PC:
			output_file << setw(width) << "R_16_PC";
			break;
		}
		output_file << setw(width) << to_string(entry.r_info);
		output_file << endl;
	}
	output_file << endl;
}

uint16_t RelocationTable::getSize()
{
	return static_cast<uint16_t>(table.size() * sizeof(ST_RELTAB_ENTRY));
}

void RelocationTable::printTable()
{
	cout << endl <<"RELOCATION TABLE: " << name << endl;
	cout << "r_offset\t" << "r_type\t" << "r_info" << endl;
	for (ST_RELTAB_ENTRY entry : table)
	{
		cout << entry.r_offset << "\t\t" << entry.r_type << "\t" << entry.r_info << endl;
	}
}
