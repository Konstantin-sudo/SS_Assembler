#pragma once
#ifndef SECTIONTABLE_H
#define SECTIONTABLE_H

#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

class SectionTable
{
public:
	SectionTable(string name_);
	
	void insertByte(int8_t data);
	
	void insertWord(int16_t data);
	
	void insertWordAtAddress(uint16_t offset, int16_t data);
	
	void addWordAtAddress(uint16_t offset, int16_t data);
	
	void writeTableToOutputTextFile(ofstream& output_file);

	uint16_t getSize();

	string int8_to_hex_string(const int8_t v);

	void printTable();

private:
	string name;
	vector<int8_t> table;

};

#endif