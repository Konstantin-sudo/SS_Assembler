#include "assembler.h"

#include <bitset>

vector<queue<string>> Assembler::tokenizedInputFile;

int Assembler::generateObjectProgramInTextFormat(string input_file_name, string output_file_name)
{
	cout << "Reading file" << endl;
	Assembler::readAndTokenizeInputFile(input_file_name);
	//testing
	/*for (queue<string> line : tokenizedInputFile) {
		int n = line.size();
		for (int i = 0; i < n;++i) {
			cout << line.front() + " ";
			line.pop();
		}
		cout << endl;
	}*/
	//
	cout << "First pass" << endl;
	firstPass();
	cout << "Resolving incalculated symbols" << endl;
	TNS_Table::resolveUndefinedSymbols();
	cout << "Backpatching" << endl;
	backpatching();
	//testing
	/*SectionHeaderTable::printTables();
	TNS_Table::printTable();*/
	//
	cout << "Writeng output file" << endl;
	writeToOutputTextFile(output_file_name);
	return 0;
}

void Assembler::readAndTokenizeInputFile(string input_file_name)
{
	ifstream input_file;
	input_file.open(input_file_name, ios::in);
	if (!input_file.is_open())
	{
		throw AsmException("Unable to open input file");
	}
	string line;
	while (getline(input_file, line))
	{
		if (line.empty())
		{
			continue;
		}
		transform(line.begin(), line.end(), line.begin(), ::tolower);		
		queue<string> tokenizedLine;
		string delimiters = ", \f\n\r\t\v";
		removeDelimitersAndCommentsFromLine(line, tokenizedLine, delimiters);
		if (!tokenizedLine.empty())
		{
			tokenizedInputFile.push_back(tokenizedLine);
			//if processedLine is equal to end_directive - ignore rest of the file
			if (tokenizedLine.front() == ".end")	
				break;
		}
	}
	if (tokenizedInputFile.empty()) 
	{
		throw AsmException("Input file is empty");
	}
	if (tokenizedInputFile.back().back() != ".end")
	{
		throw AsmException("Input file must end with '.end' directive");
	}
	input_file.close();
}

void Assembler::removeDelimitersAndCommentsFromLine(const string& line, queue<string>& tokenizedLine, const string& delimiters)
{
	size_t pos_char = line.find_first_not_of(delimiters, 0);
	size_t pos_white = line.find_first_of(delimiters, pos_char);
	while (pos_char != string::npos || pos_white != string::npos)
	{
		string substring = line.substr(pos_char, pos_white - pos_char);
		size_t comment_char_index = substring.find("#", 0);
		if (comment_char_index != string::npos)
		{
			substring = substring.substr(0, comment_char_index);
			if (!substring.empty())
			{
				tokenizedLine.push(substring);
			}
			break;
		}
		tokenizedLine.push(substring);		
		pos_char = line.find_first_not_of(delimiters, pos_white);
		pos_white = line.find_first_of(delimiters, pos_char);
	}
}

void Assembler::firstPass()
{
	SectionType currSection = SectionType::NO_SECTION;
	string curr_section_string = "";
	uint16_t LC = 0; //location counter

	unsigned long numberOfLinesInInputFile = tokenizedInputFile.size();
	for (unsigned long lineNo = 0; lineNo < numberOfLinesInInputFile; ++lineNo)
	{
		queue<string> lineOfTokens = tokenizedInputFile[lineNo];

		Token currToken = Token::parseToken(lineOfTokens.front());
		lineOfTokens.pop();

		if (currToken.getType() == TokenType::LABEL)
		{
			if (currSection == SectionType::NO_SECTION)
			{
				throw AsmException("Label: '" + currToken.getValue() + "' must be defined in section. Error at line " + to_string(lineNo + 1));
			}
			SectionHeaderTable::getSymbolTable()->insertSymbol(currToken, LC, curr_section_string, lineNo);
			//cout << endl << "FOUND LABEL ";
			if (lineOfTokens.size() != 0)
			{
				currToken = Token::parseToken(lineOfTokens.front());
				lineOfTokens.pop();
			}
			else
			{
				++lineNo;
				lineOfTokens = tokenizedInputFile[lineNo];
				currToken = Token::parseToken(lineOfTokens.front());
				lineOfTokens.pop();
				//continue;
			}
		}

		switch (currToken.getType())
		{
		case TokenType::LABEL:
		{
			throw AsmException("Label: '" + currToken.getValue() + "' can't come after label. Error at line " + to_string(lineNo + 1));
			break;
		}
		case TokenType::INSTRUCTION:
		{
			if (currSection != SectionType::TEXT && currSection != SectionType::USER_SECTION)
			{
				throw AsmException("Instruction: '" + currToken.getValue() + "' must be in .text section or user defined section. Error at line " + to_string(lineNo + 1));
			}
			string mnemonik = currToken.getValue();
			LC += Instruction::processInstruction(mnemonik, lineOfTokens, curr_section_string, LC, lineNo);
			
			//cout << endl << "FOUND INSTRUCTION";
			break;
		}
		case TokenType::ACCESS_RIGHTS_DIRECTIVE:
		{
			string access_right = currToken.getValue();
			if (lineOfTokens.size() == 0)
			{
				throw AsmException("Directive '" + access_right + "' can only be followed with one or more symbols. Error at line " + to_string(lineNo + 1));
			}
			unsigned long symbol_cnt = lineOfTokens.size();
			for (unsigned long k = 0; k < symbol_cnt; k++)
			{
				currToken = Token::parseToken(lineOfTokens.front());
				lineOfTokens.pop();

				if (currToken.getType() == TokenType::SYMBOL)
				{
					SectionHeaderTable::getSymbolTable()->updateAccessRights(currToken.getValue(), access_right, lineNo);
				}
				else
				{
					throw AsmException(currToken.getValue() + " is not symbol. Directive '" + access_right + "' must be followed with symbols. Error at line " + to_string(lineNo + 1));
				}
			}

			//cout << endl << "FOUND ACCESS_RIGHTS_DIRECTIVE ";
			break;
		}
		case TokenType::MEMORY_ALLOCATION_DIRECTIVE: 
		{
			if (currSection == NO_SECTION)
			{
				throw AsmException("Directive '" + currToken.getValue() + "' must be defined in section. Error at line " + to_string(lineNo + 1));
			}
			if (lineOfTokens.size() == 0)
			{
				throw AsmException("Directive '" + currToken.getValue() + "' can only be followed with one or more symbol/literal. Error at line " + to_string(lineNo + 1));
			}
			if (currToken.getValue() == ".skip")
			{
				unsigned long numberOfTokensInLine = lineOfTokens.size();
				for (unsigned long k = 0; k < numberOfTokensInLine; k++)
				{
					currToken = Token::parseToken(lineOfTokens.front());
					lineOfTokens.pop();

					if (currToken.getType() == TokenType::DEC_NUMBER || currToken.getType() == TokenType::HEX_NUMBER)
					{
						unsigned long numberOfBytes = stol(currToken.getValue(), NULL, 0);
						if (numberOfBytes > 65536) //size of all memory = 64KB = 65536 B
						{
							throw AsmException("Not enough memory. Number '" + currToken.getValue() + "' at '.skip' directive is to big. Error at line " + to_string(lineNo + 1));
						}
						if (numberOfBytes < 0)
						{
							throw AsmException(".skip directive accepts only positive numbers. Error at line " + to_string(lineNo + 1));
						}
						for (unsigned long m = 0; m < numberOfBytes; ++m)
						{
							SectionHeaderTable::getSectionTable(curr_section_string)->insertByte(0);
						}
						LC += static_cast<uint16_t>(numberOfBytes);
					}
					else
					{
						throw AsmException(currToken.getValue() + " is not positive decimal or hex number. Error at line " + to_string(lineNo + 1));
					}
				}
			}
			else
			{
				if (currSection == SectionType::BSS)
				{
					throw AsmException("Section .bss doesn't allow '" + currToken.getValue() + "'. directive. Error at line " + to_string(lineNo + 1));
				}
				if (currToken.getValue() == ".byte")
				{
					unsigned long numberOfTokensInLine = lineOfTokens.size();
					for (unsigned long k = 0; k < numberOfTokensInLine; k++)
					{
						currToken = Token::parseToken(lineOfTokens.front());
						lineOfTokens.pop();

						switch (currToken.getType())
						{
							case TokenType::DEC_NUMBER:
							{
								long value = stol(currToken.getValue(), NULL, 0);
								if (value > 127 || value < -127)
								{
									throw AsmException("Number '" + currToken.getValue() + "' at directive '.byte' must be in range [-127,127]. Error at line " + to_string(lineNo + 1));
								}
								int8_t data = value & 0x00FF;
								SectionHeaderTable::getSectionTable(curr_section_string)->insertByte(data);
								LC += 1;
								break;
							}
							case TokenType::HEX_NUMBER:
							{
								long value = stoul(currToken.getValue(), NULL, 0);;
								if (value > 255)
								{
									throw AsmException("Number '" + currToken.getValue() + "' at directive '.byte' must be in range [0x00,0xFF]. Error at line " + to_string(lineNo + 1));
								}
								int8_t data = value & 0x00FF;
								SectionHeaderTable::getSectionTable(curr_section_string)->insertByte(data);
								LC += 1;
								break;
							}
							default:
							{
								throw AsmException(currToken.getValue()+" is not symbol/literal. Directive .byte must be followed with /*symbol or*/ literal. Error at line " + to_string(lineNo + 1));
								break;
							}
						}
					}
				}
				else if (currToken.getValue() == ".word")
				{
					unsigned long numberOfTokensInLine = lineOfTokens.size();
					for (unsigned long k = 0; k < numberOfTokensInLine; k++)
					{
						currToken = Token::parseToken(lineOfTokens.front());
						lineOfTokens.pop();
						switch (currToken.getType())
						{
							case TokenType::DEC_NUMBER:
							{
								long value = stol(currToken.getValue(), NULL, 0);;
								if (value > 32767 || value < -32767)
								{
									throw AsmException("Number '" + currToken.getValue() + "' at directive '.word' must be in range [-32767,32767]. Error at line " + to_string(lineNo + 1));
								}
								int16_t data = value & 0xFFFF;
								SectionHeaderTable::getSectionTable(curr_section_string)->insertWord(data);
								LC += 2;
								break;
							}
							case TokenType::HEX_NUMBER:
							{
								long value = stol(currToken.getValue(), NULL, 0);;
								if (value > 65535)
								{
									throw AsmException("Number '" + currToken.getValue() + "' at directive '.word' must be in range [0x0000, 0xFFFF]. Error at line " + to_string(lineNo + 1));
								}
								SectionHeaderTable::getSectionTable(curr_section_string)->insertWord(value & 0xFFFF);
								LC += 2;
								break;
							}
							case TokenType::SYMBOL:
							{
								uint16_t value = SectionHeaderTable::getSymbolTable()->getSymbolValueAndCreateRelTableEntry(currToken.getValue(), LC, curr_section_string, rel_type::R_16);
								SectionHeaderTable::getSectionTable(curr_section_string)->insertWord(value);
								LC += 2;
								break;
							}
							case TokenType::SECTION_DIRECTIVE:
							{
								if (currToken.getValue() == ".section")
								{
									throw AsmException(currToken.getValue() + " is not symbol/literal. Directive .word must be followed with symbol or literal. Error at line " + to_string(lineNo + 1));
								}
								uint16_t value = SectionHeaderTable::getSymbolTable()->getSymbolValueAndCreateRelTableEntry(currToken.getValue(), LC, curr_section_string, rel_type::R_16);
								SectionHeaderTable::getSectionTable(curr_section_string)->insertWord(value);
								LC += 2;
								break;
							}
							default:
							{
								throw AsmException(currToken.getValue() + " is not symbol/literal. Directive .word must be followed with symbol or literal. Error at line " + to_string(lineNo + 1));
								break;
							}
						}
					}
				}
				
			}

			//cout << endl << "FOUND MEMORY_ALLOCATION_DIRECTIVE";
			break;
		}
		case TokenType::SECTION_DIRECTIVE:
		{
			if (currToken.getValue() != ".section")
				throw AsmException("Name of section " + currToken.getValue() + " must follow directive '.section'. Error at line " + to_string(lineNo + 1));

			if (lineOfTokens.size() != 0) 
			{
				currToken = Token::parseToken(lineOfTokens.front());
				lineOfTokens.pop();

				if (currToken.getType() == TokenType::SECTION_DIRECTIVE && currToken.getValue() != ".section")
				{
					SectionHeaderTable::getSymbolTable()->updateSymbolSize(curr_section_string, LC);
					LC = 0;

					curr_section_string = currToken.getValue();
					if (curr_section_string == ".text")
					{
						currSection = SectionType::TEXT;
					}
					else if (curr_section_string == ".bss")
					{
						currSection = SectionType::BSS;
					}
					else if (curr_section_string == ".data")
					{
						currSection = SectionType::DATA;
					}
					else if (curr_section_string == ".rodata")
					{
						currSection = SectionType::RODATA;
					}
					else
					{
						currSection = SectionType::USER_SECTION;
					}
					
					SectionHeaderTable::getSymbolTable()->insertSymbol(currToken, LC, curr_section_string, lineNo);
					
					if (lineOfTokens.size() != 0)
					{
						throw AsmException("Incorect syntax. Too many parameters at directive .section. Error at line " + to_string(lineNo + 1));
					}
				}
				else
				{
					throw AsmException(currToken.getValue() + " is invalid name of section. Error at line " + to_string(lineNo + 1));
				}
			}
			else 
			{
				throw AsmException("Section directive '.section' must be followed by name of section. Error at line " + to_string(lineNo + 1));
			}
			//cout << endl << "FOUND SECTION_DIRECTIVE";
			break;
		}
		case TokenType::EQU_DIRECTIVE:
		{
			if (currSection == SectionType::NO_SECTION)
			{
				throw AsmException("'.equ' directive must be in section. Error at line: " + to_string(lineNo + 1));
			}
			//da li vrednost simbola moze da bude negativna ? da li postoje ogranicenja u sekcijama ? da li vrednost simbola moze da bude veca od velicine sekcije u kojoj je definisan ? da li treba da se racuna indeks klasifikacije ?
			if (lineOfTokens.size() < 2)
			{
				throw AsmException("'.equ' directive must have parameters. Error at line: " + to_string(lineNo + 1));
			}
			Token new_symbol_token = Token::parseToken(lineOfTokens.front());
			lineOfTokens.pop();
			if (new_symbol_token.getType() != TokenType::SYMBOL)
			{
				throw AsmException(".equ directive can only define new symbols. " + new_symbol_token.getValue() + " is not symbol . Error at line: " + to_string(lineNo + 1));
			}
			//prepare for calculation
			string expression_str = Arithmetic::formAndValidateExpressionString(lineOfTokens, new_symbol_token.getValue(), lineNo);
			queue<Token> queueOfArithmeticTokens;
			Arithmetic::parseExpression(expression_str, queueOfArithmeticTokens);
			//calculate
			bool succses = false;
			uint16_t expression_value = Arithmetic::calculateExpression(queueOfArithmeticTokens, succses, new_symbol_token.getValue(), lineNo);
			if (succses)
			{
				SectionHeaderTable::getSymbolTable()->insertSymbol(new_symbol_token, expression_value, curr_section_string, lineNo);
			}
			else
			{	
				//if failed to calculate
				SectionHeaderTable::getSymbolTable()->insertSymbol(new_symbol_token, 0, curr_section_string, lineNo, false);
				//create entry in tns table
				TNS_Table::insert(new_symbol_token.getValue(), queueOfArithmeticTokens);
			}

			//cout << endl << "FOUND EQU_DIRECTIVE " << new_symbol_token.getValue() + ", " + expression_str;
			break;
		}
		case TokenType::END_DIRECTIVE:
		{
			SectionHeaderTable::getSymbolTable()->updateSymbolSize(curr_section_string, LC);
			LC = 0;
			//cout << endl << "FOUND END DIRECTIVE ";
			break;
		}
		default:
		{
			throw AsmException("Unrecognized token: " + currToken.getValue() + " at line: " + to_string(lineNo + 1));
			break;
		}
		}//end switch
	}//end for
	
}

void Assembler::backpatching()
{
	SectionHeaderTable::getSymbolTable()->backpatching();
}

void Assembler::writeToOutputTextFile(string output_file_name)
{
	ofstream output_file;
	output_file.open(output_file_name, ios::out);
	if (!output_file.is_open())
	{
		throw AsmException("Unable to open output file");
	}
	SectionHeaderTable::writeTablesToOutputTextFile(output_file);
	TNS_Table::writeTableToOutputTextFile(output_file);
	output_file.close();
}
