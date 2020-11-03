#include "sectionTable.h"
#include <bitset>

SectionTable::SectionTable(string name_)
{
	name = name_;
}

void SectionTable::insertByte(int8_t data)
{
	table.push_back(data);
}

void SectionTable::insertWord(int16_t data)
{
	int8_t data_low = data & 0x00FF;
	int8_t data_high = (data & 0xFF00) >> 8;
	table.push_back(data_low);
	table.push_back(data_high);
}

void SectionTable::insertWordAtAddress(uint16_t offset, int16_t data)
{
	for (unsigned long i = 0; i < table.size(); ++i)
	{
		if (offset == i)
		{
			table[i]= data & 0x00FF;
			table[i + 1] = (data & 0xFF00) >> 8;
			break;
		}
	}
}

void SectionTable::addWordAtAddress(uint16_t offset, int16_t data)
{
	for (unsigned long i = 0; i < table.size(); ++i)
	{
		if (offset == i)
		{
			int16_t existing_data = table[i + 1];
			existing_data <<= 8;
			existing_data |= table[i];
			existing_data += data;
			table[i] = existing_data & 0x00FF;
			table[i + 1] = (existing_data & 0xFF00) >> 8;
			break;
		}
	}
}

void SectionTable::writeTableToOutputTextFile(ofstream& output_file)
{
	if (name != ".bss")
	{
		const unsigned short width = 20;
		output_file << endl << "//-----------------------------SECTION " + name + "-----------------------------\\\\" << endl << endl;

		const unsigned long MAX_BYTES_IN_LINE_CNT = 25;
		unsigned long bytes_in_line_cnt = 0;
		for (int8_t byte : table)
		{
			if (bytes_in_line_cnt == MAX_BYTES_IN_LINE_CNT)
			{
				output_file << endl;
				bytes_in_line_cnt = 0;
			}
			output_file << setw(2) << int8_to_hex_string(byte) << " ";
			++bytes_in_line_cnt;
			//bitset<8> x(byte);
		}
		output_file << endl << endl;
	}
}

uint16_t SectionTable::getSize()
{
	return static_cast<uint16_t>(table.size());
}

string SectionTable::int8_to_hex_string(const int8_t b)
{
	stringstream ss;
	uint8_t byte = b;
	ss << hex << ((byte >> 4) & 0xF);
	ss << hex << (byte & 0xF);
	return ss.str();
}

void SectionTable::printTable()
{
	cout << endl << "SECTION " << name << endl;
	
	for (int8_t byte : table)
	{
		bitset<8> x(byte);
		cout << x << " ";
	}
	cout << endl;
}
