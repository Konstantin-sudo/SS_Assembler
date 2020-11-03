#include "instruction.h"

#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#endif

map<string, ST_INSTRUCTION_INFO> Instruction::instructions_hash_map = {
	//
	{ "halt", ST_INSTRUCTION_INFO(0,0,mnemonik_type::TYPE_NULL) },
	{ "iret", ST_INSTRUCTION_INFO(1,0,mnemonik_type::TYPE_NULL) },
	{ "ret", ST_INSTRUCTION_INFO(2,0,mnemonik_type::TYPE_NULL) },

	{ "int", ST_INSTRUCTION_INFO(3,1,mnemonik_type::TYPE_JMP) },
	{ "call", ST_INSTRUCTION_INFO(4,1,mnemonik_type::TYPE_JMP) },
	//jump instructions
	{ "jmp", ST_INSTRUCTION_INFO(5,1,mnemonik_type::TYPE_JMP) },
	{ "jeq", ST_INSTRUCTION_INFO(6,1,mnemonik_type::TYPE_JMP) },
	{ "jne", ST_INSTRUCTION_INFO(7,1,mnemonik_type::TYPE_JMP) },
	{ "jgt", ST_INSTRUCTION_INFO(8,1,mnemonik_type::TYPE_JMP) },
	//stack instructions
	{ "push", ST_INSTRUCTION_INFO(9,1,mnemonik_type::TYPE_DATA) },
	{ "pop", ST_INSTRUCTION_INFO(10,1,mnemonik_type::TYPE_DATA) },
	//
	{ "xchg", ST_INSTRUCTION_INFO(11,2,mnemonik_type::TYPE_DATA) },
	{ "mov", ST_INSTRUCTION_INFO(12,2,mnemonik_type::TYPE_DATA) },
	//aritmetic instructions
	{ "add", ST_INSTRUCTION_INFO(13,2,mnemonik_type::TYPE_DATA) },
	{ "sub", ST_INSTRUCTION_INFO(14,2,mnemonik_type::TYPE_DATA) },
	{ "mul", ST_INSTRUCTION_INFO(15,2,mnemonik_type::TYPE_DATA) },
	{ "div", ST_INSTRUCTION_INFO(16,2,mnemonik_type::TYPE_DATA) },
	{ "cmp", ST_INSTRUCTION_INFO(17,2,mnemonik_type::TYPE_DATA) },
	//logic instructions
	{ "not", ST_INSTRUCTION_INFO(18,2,mnemonik_type::TYPE_DATA) },
	{ "and", ST_INSTRUCTION_INFO(19,2,mnemonik_type::TYPE_DATA) },
	{ "or", ST_INSTRUCTION_INFO(20,2,mnemonik_type::TYPE_DATA) },
	{ "xor", ST_INSTRUCTION_INFO(21,2,mnemonik_type::TYPE_DATA) },
	{ "test", ST_INSTRUCTION_INFO(22,2,mnemonik_type::TYPE_DATA) },
	{ "shl", ST_INSTRUCTION_INFO(23,2,mnemonik_type::TYPE_DATA) },
	{ "shr", ST_INSTRUCTION_INFO(24,2,mnemonik_type::TYPE_DATA) }

};

map<addressing_mode, int8_t> Instruction::addressing_mode_hash_map = {
	{ addressing_mode::IMMEDIATE, 0x00 },								//Op1descr byte format: AM2 AM1 AM0 R3 R2 R1 R0 L/H
	{ addressing_mode::REGISTER_DIRECT, 0x01 },
	{ addressing_mode::REGISTER_INDIRECT, 0x02 },
	{ addressing_mode::REGISTER_INDIRECT_WITH_OFFSET, 0x03 },
	{ addressing_mode::MEMORY_DIRECT, 0x04 }
};

uint16_t Instruction::processInstruction(string mnemonik, queue<string> queue_of_operands, string currSection_string, unsigned long LC, unsigned long lineNo)
{
	//opernad size
	uint16_t operand_size = 2; // = 2B
	bool explicit_word_operand_size = false;
	if (mnemonik.at(mnemonik.size() - 1) == 'b' && mnemonik != "sub") //sub
	{
		mnemonik = mnemonik.substr(0, mnemonik.size() - 1);
		operand_size = 1; // = 1B
	}
	else if (mnemonik.at(mnemonik.size() - 1) == 'w')
	{
		explicit_word_operand_size = true;
		mnemonik = mnemonik.substr(0, mnemonik.size() - 1);
	}

	//mnemonik
	auto mnemonik_details = instructions_hash_map.find(mnemonik);
	if (mnemonik_details == instructions_hash_map.end())
	{
		throw AsmException("Unrecognized instruction: '" + mnemonik + "'.Error at line " + to_string(lineNo + 1)); //should never happen
	}
	if (queue_of_operands.size() != mnemonik_details->second.number_of_operands)
	{
		throw AsmException("Invalid number of operands at instruction: '" + mnemonik + "'.Error at line " + to_string(lineNo + 1));
	}

	vector<int8_t> instruction_code;

	//InstrDescr byte: (OC4 OC3 OC2 OC1 OC0 S Un Un)
	int8_t byte_1 = mnemonik_details->second.op_code << 3;

	switch (operand_size)										//halt,iret,ret ??? 0 ili 1 ?
	{
	case 1:
		byte_1 |= 0x00;
		break;
	case 2:
		byte_1 |= 0x04;
		break;
	}
	instruction_code.push_back(byte_1);

	//Op<1/2>Descr byte: (AM2 AM1 AM0 R3 R2 R1 R0 L/H) + Im/Di/Ad byte + Im/Di/Ad byte 
	int8_t byte_2 = 0x00;
	for (uint16_t i = 0; i < mnemonik_details->second.number_of_operands; ++i)
	{
		Token operand_token = Token::parseToken(queue_of_operands.front());
		queue_of_operands.pop();
		string operand = operand_token.getValue();
		switch (operand_token.getType())
		{
		case TokenType::OPERAND_IMMEDIATE_HEX_DATA:
		case TokenType::OPERAND_IMMEDIATE_DEC_DATA:
		case TokenType::OPERAND_IMMEDIATE_SYMBOL_DATA:
		{
			if (mnemonik_details->second.type != mnemonik_type::TYPE_DATA)
			{
				throw AsmException("Invalid syntax at operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
			}
			if (i == 1 && mnemonik_details->second.number_of_operands == 2 && mnemonik != "cmp" && mnemonik != "test")
			{
				throw AsmException("Immediate operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "' is not allowed as destination operand. Error at line " + to_string(lineNo + 1));
			}

			//addressing
			byte_2 = 0;
			auto addressing_mode_code_search = addressing_mode_hash_map.find(addressing_mode::IMMEDIATE);
			byte_2 |= addressing_mode_code_search->second << 5;
			instruction_code.push_back(byte_2);

			//operand value
			string value_str = operand.substr(1, operand.size() - 1); // without '$' : $0xFA45 -> 0xFA45 or $b -> b
			long value;
			if (operand_token.getType() != TokenType::OPERAND_IMMEDIATE_SYMBOL_DATA)
			{
				value = stol(value_str, NULL, 0);
				if (value > 65535 && operand_token.getType() == TokenType::OPERAND_IMMEDIATE_HEX_DATA)
				{
					throw AsmException("Number '" + value_str + "' at instruction '" + mnemonik + "' must be in range [0x0000, 0xFFFF]. Error at line " + to_string(lineNo + 1));
				}
				else if ((value > 32767 || value < -32767) && operand_token.getType() == TokenType::OPERAND_IMMEDIATE_DEC_DATA)
				{
					throw AsmException("Number '" + value_str + "' at instruction '" + mnemonik + "' must be in range [-32767, 32767]. Error at line " + to_string(lineNo + 1));
				}
				if ( ((value >= -127 && value <= 127) && operand_token.getType() == TokenType::OPERAND_IMMEDIATE_DEC_DATA) || (value <= 255 && operand_token.getType() == TokenType::OPERAND_IMMEDIATE_HEX_DATA)) // 1B enough for immediate value 
				{
					if (explicit_word_operand_size == true)
					{
						int8_t data_low = value & 0x00FF;
						int8_t data_high = (value & 0xFF00) >> 8;
						instruction_code.push_back(data_low);
						instruction_code.push_back(data_high);
					}
					else
					{
						int8_t data_low = value & 0x00FF;
						instruction_code.push_back(data_low);
					}
				}
				else // 2B for value 
				{
					if (operand_size == 1)
					{
						throw AsmException("Number '" + value_str + "' at '" + mnemonik + "b' is too big for one byte. Error at line " + to_string(lineNo + 1));
					}
					int8_t data_low = value & 0x00FF;
					int8_t data_high = (value & 0xFF00) >> 8;
					instruction_code.push_back(data_low);
					instruction_code.push_back(data_high);
				}
			}
			else 
			{
				if (operand_size == 1)
				{
					throw AsmException("Suffix 'b' at instruction '" + mnemonik + "' can't be paired with immediate symbol value(always 2B). Error at line " + to_string(lineNo + 1));
				}
				value = SectionHeaderTable::getSymbolTable()->getSymbolValueAndCreateRelTableEntry(value_str, static_cast<uint16_t>(LC + static_cast<uint16_t>(instruction_code.size())), currSection_string, rel_type::R_16);
				int8_t data_low = value & 0x00FF;
				int8_t data_high = (value & 0xFF00) >> 8;
				instruction_code.push_back(data_low);
				instruction_code.push_back(data_high);
			}
			
			
			
			break;
		}
		case TokenType::OPERAND_REGISTER_DIRECT_ADDRESSING_DATA:
		{
			if (mnemonik_details->second.type != mnemonik_type::TYPE_DATA)
			{
				throw AsmException("Invalid syntax at operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
			}

			//addressing
			byte_2 = 0;
			auto addressing_mode_code_search = addressing_mode_hash_map.find(addressing_mode::REGISTER_DIRECT);
			byte_2 |= addressing_mode_code_search->second << 5;
			//l/h bit
			if (operand.at(operand.size() - 1) == 'h')
			{
				if (operand_size != 1)
				{
					throw AsmException("Invalid syntax. Missing suffix 'b' at intruction '" + mnemonik + "' for operand '" + operand_token.getValue() + "h'. Error at line " + to_string(lineNo + 1));
				}
				operand = operand.substr(0, operand.size() - 1);
				byte_2 |= 0x01;
			}
			else if (operand_token.getValue().at(operand_token.getValue().size() - 1) == 'l')
			{
				if (operand_size != 1)
				{
					throw AsmException("Invalid syntax. Missing suffix 'b' at intruction '" + mnemonik + "' for operand '" + operand_token.getValue() + "l'. Error at line " + to_string(lineNo + 1));
				}
				operand = operand.substr(0, operand.size() - 1);
				byte_2 |= 0x00;
			}
			//reg code
			string register_number_str;
			if (operand.size() == 3) // %r0-7
			{
				register_number_str = operand.substr(operand.size() - 1, 1);
			}
			else //%r15 == psw
			{
				register_number_str = operand.substr(operand.size() - 2, 2);
			}
			int8_t register_number = stoi(register_number_str, NULL, 0); // register_number  = [0,7]
			byte_2 |= register_number << 1;

			instruction_code.push_back(byte_2);

			break;
		}
		case TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_DATA:
		{
			if (mnemonik_details->second.type != mnemonik_type::TYPE_DATA)
			{
				throw AsmException("Invalid syntax at operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
			}

			//addressing
			byte_2 = 0;
			auto addressing_mode_code_search = addressing_mode_hash_map.find(addressing_mode::REGISTER_INDIRECT);
			byte_2 |= addressing_mode_code_search->second << 5;
			//reg code
			string register_number_str;
			if (operand.size() == 5) //	(%r7)
			{
				register_number_str = operand.substr(operand.size() - 2, 1);
			}
			else //(%r15) == psw
			{
				register_number_str = operand.substr(operand.size() - 3, 2);
			}
			int8_t register_number = stoi(register_number_str, NULL, 0); // register_number  = [0,7]
			byte_2 |= register_number << 1;
			
			instruction_code.push_back(byte_2);
			
			break;
		}
		case TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_WITH_OFFSET_DATA:
		{
			if (mnemonik_details->second.type != mnemonik_type::TYPE_DATA)
			{
				throw AsmException("Invalid syntax at operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
			}
			
			//addressing
			byte_2 = 0;
			auto addressing_mode_code_search = addressing_mode_hash_map.find(addressing_mode::REGISTER_INDIRECT_WITH_OFFSET);
			byte_2 |= addressing_mode_code_search->second << 5;
			//reg code
			//da li psw registar sme da se koristi za ovo ??????????????????????????????????
			size_t percentage_char_pos = operand.find('%', 0);
			string register_number_str;
			if (operand.size() - percentage_char_pos - 1 == 3 ) //symbol(%r7) 11-7-1=3
			{
				register_number_str = operand.substr(percentage_char_pos + 2, 1);
			}
			else // symbol(%r15) == psw
			{
				register_number_str = operand.substr(percentage_char_pos + 2, 2);
			}
			int8_t register_number = stoi(register_number_str, NULL, 0); // register_number  = [0,7]
			byte_2 |= register_number << 1;
			instruction_code.push_back(byte_2);

			//opernad value
			size_t brackets_open_pos = operand.find('(', 0);
			string offset_str = operand.substr(0, operand.size() - (operand.size() - brackets_open_pos));
			long offset = 0;
			Token offset_token = Token::parseToken(offset_str);
			switch (offset_token.getType())
			{
			case TokenType::DEC_NUMBER: 
			{
				offset = stol(offset_str, NULL, 0); 
				if (offset > 32767 || offset < -32767)
				{
					throw AsmException("Number '" + offset_str + "' at instruction '" + mnemonik + "' must be in range [-32767, 32767]. Error at line " + to_string(lineNo + 1));
				}
				break;
			}
			case TokenType::HEX_NUMBER: 
			{
				offset = stol(offset_str, NULL, 0);
				if (offset > 65535)
				{
					throw AsmException("Number '" + offset_str + "' at instruction '" + mnemonik + "' must be in range [0x0000, 0xFFFF]. Error at line " + to_string(lineNo + 1));
				}
				break;
			}
			case TokenType::SYMBOL: 
			{
				rel_type relocation_type = rel_type::R_16;
				if (register_number == 7) //offset = address(symbol) - PC = S(symbol) + offset(symbol) - * - 2;  * == address_of_this_offset_in_curr_section
				{
					if (SectionHeaderTable::getSymbolTable()->getSectionIndex(currSection_string) == SectionHeaderTable::getSymbolTable()->getIndexOfSectionWhereSymbolIsDefined(offset_str))
					{
						int size_of_this_instruction = 0;
						//if this is first operand
						if (i == 0) //this is first opreand
						{						
							size_of_this_instruction += 1; // OC
							size_of_this_instruction += 3; // this operand
							//size of next operand
							Token next_operand = Token::parseToken(queue_of_operands.front());
							switch (next_operand.getType())
							{
							case TokenType::OPERAND_IMMEDIATE_DEC_DATA:
							case TokenType::OPERAND_IMMEDIATE_HEX_DATA:
							{
								string value_str = next_operand.getValue().substr(1, operand.size() - 1); // without '$' : $0xFA45 -> 0xFA45 or $b -> b
								int16_t value = stoi(value_str, NULL, 0);
								if (value >= -127 && value <= 127) // 1B enough for immediate value 
								{
									size_of_this_instruction += 2;
								}
								else // 2B for value 
								{
									size_of_this_instruction += 3;
								}
							
								break;
							}
							case TokenType::OPERAND_IMMEDIATE_SYMBOL_DATA:
							case TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_WITH_OFFSET_DATA:
							case TokenType::DEC_NUMBER:
							case TokenType::HEX_NUMBER:
							case TokenType::SYMBOL:
							{
								size_of_this_instruction += 3; 
								break;
							}
							case TokenType::OPERAND_REGISTER_DIRECT_ADDRESSING_DATA:
							case TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_DATA:
							{
								size_of_this_instruction += 1;
								break;
							}
							default:
								throw AsmException("Invalid syntax at operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
								break;
							}

						}
						else
						{
							//if this is second operand
							size_of_this_instruction += instruction_code.size() + 2;
						}
						offset = LC + size_of_this_instruction; // == -PC  //no rel table 
						offset *= -1;
					}
					else
					{
						if (i == 0)
						{
							int size_of_next_operand = 0;
							//size of next operand
							Token next_operand = Token::parseToken(queue_of_operands.front());
							switch (next_operand.getType())
							{
							case TokenType::OPERAND_IMMEDIATE_DEC_DATA:
							case TokenType::OPERAND_IMMEDIATE_HEX_DATA:
							{
								string value_str = next_operand.getValue().substr(1, operand.size() - 1); // without '$' : $0xFA45 -> 0xFA45 or $b -> b
								int16_t value = stoi(value_str, NULL, 0);
								if (value >= -127 && value <= 127) // 1B enough for immediate value 
								{
									size_of_next_operand += 2;
								}
								else // 2B for value 
								{
									size_of_next_operand += 3;
								}

								break;
							}
							case TokenType::OPERAND_IMMEDIATE_SYMBOL_DATA:
							case TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_WITH_OFFSET_DATA:
							case TokenType::DEC_NUMBER:
							case TokenType::HEX_NUMBER:
							case TokenType::SYMBOL:
							{
								size_of_next_operand += 3;
								break;
							}
							case TokenType::OPERAND_REGISTER_DIRECT_ADDRESSING_DATA:
							case TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_DATA:
							{
								size_of_next_operand += 1;
								break;
							}
							default:
								throw AsmException("Invalid syntax at operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
								break;
							}

							offset -= (2 + size_of_next_operand);
						}
						else
						{
							offset -= 2;
						}
						// for linker : S(symbol) - * ; S(symbol) == address of start of the section, * == LC + 2 (address of this offset)  - SYMBOL_LOCAL
						// for linker : address(symbol) - * ; addres(symbol) == address of the symbol, * == LC + 2 (address of this offset)  - SYMBOL_GLOBAL
					}
					relocation_type = rel_type::R_16_PC;
				}
				offset += SectionHeaderTable::getSymbolTable()->getSymbolValueAndCreateRelTableEntry(offset_str, static_cast<uint16_t>(LC + static_cast<uint16_t>(instruction_code.size())), currSection_string, relocation_type);
				break;
			}
			default:
				throw AsmException("Invalid syntax at operand '" + operand + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
				break;
			}
			int8_t data_low = offset & 0x00FF;
			int8_t data_high = (offset & 0xFF00) >> 8;
			instruction_code.push_back(data_low);
			instruction_code.push_back(data_high);

			break;
		}
		case TokenType::DEC_NUMBER:		//operand absolute dec_data || operand immediate dec_jmp
		case TokenType::HEX_NUMBER:		//operand absolute hex_data || operand immediate hex_jmp
		case TokenType::SYMBOL:			//operand absolute symbol_data || operand_immediate_symbol_jmp
		{
			switch (mnemonik_details->second.type)
			{
			case mnemonik_type::TYPE_DATA:
			{
				//addressing
				byte_2 = 0;
				auto addressing_mode_code_search = addressing_mode_hash_map.find(addressing_mode::MEMORY_DIRECT);
				byte_2 |= addressing_mode_code_search->second << 5;
				instruction_code.push_back(byte_2);

				//operand value (address)
				long value;
				if (operand_token.getType() != TokenType::SYMBOL)
				{
					value = stol(operand, NULL, 0);
					if (value > 65535 && operand_token.getType() == TokenType::HEX_NUMBER)
					{
						throw AsmException("Number '" + operand + "' at instruction '" + mnemonik + "' must be in range [0x0000, 0xFFFF] (address). Error at line " + to_string(lineNo + 1));
					}
					else if ((value > 65535 || value < 0) && operand_token.getType() == TokenType::DEC_NUMBER)
					{
						throw AsmException("Number '" + operand + "' at instruction '" + mnemonik + "' must be in range [0, 65535] (address). Error at line " + to_string(lineNo + 1));
					}
				}
				else
				{
					value = SectionHeaderTable::getSymbolTable()->getSymbolValueAndCreateRelTableEntry(operand, static_cast<uint16_t>(LC + static_cast<uint16_t>(instruction_code.size())), currSection_string, rel_type::R_16);
				}
				int8_t data_low = value & 0x00FF;
				int8_t data_high = (value & 0xFF00) >> 8;
				instruction_code.push_back(data_low);
				instruction_code.push_back(data_high);
				
				break;
			}
			case mnemonik_type::TYPE_JMP:
			{
				//addressing
				byte_2 = 0;
				auto addressing_mode_code_search = addressing_mode_hash_map.find(addressing_mode::IMMEDIATE);
				byte_2 |= addressing_mode_code_search->second << 5;
				instruction_code.push_back(byte_2);

				//operand value
				long value;
				if (operand_token.getType() != TokenType::SYMBOL)
				{
					value = stol(operand, NULL, 0);
					if (value > 65535 && operand_token.getType() == TokenType::HEX_NUMBER)
					{
						throw AsmException("Number '" + operand + "' at instruction '" + mnemonik + "' must be in range [0x0000, 0xFFFF]. Error at line " + to_string(lineNo + 1));
					}
					else if (value > 65535 || value < 0 && operand_token.getType() == TokenType::DEC_NUMBER)
					{
						throw AsmException("Number '" + operand + "' at instruction '" + mnemonik + "' must be in range [0, 65535] (address). Error at line " + to_string(lineNo + 1));
					}
					if (value >= -127 && value <= 127) // 1B enough for immediate value 
					{
						int8_t data_low = value & 0x00FF;
						instruction_code.push_back(data_low);
					}
					else // 2B for value 
					{
						int8_t data_low = value & 0x00FF;
						int8_t data_high = (value & 0xFF00) >> 8;
						instruction_code.push_back(data_low);
						instruction_code.push_back(data_high);
					}
				}
				else
				{
					value = SectionHeaderTable::getSymbolTable()->getSymbolValueAndCreateRelTableEntry(operand, static_cast<uint16_t>(LC + static_cast<uint16_t>(2)), currSection_string, rel_type::R_16);
					int8_t data_low = value & 0x00FF;
					int8_t data_high = (value & 0xFF00) >> 8;
					instruction_code.push_back(data_low);
					instruction_code.push_back(data_high);
				}

				break;
			}
			}
			break;
		}
		case TokenType::OPERAND_REGISTER_DIRECT_ADDRESSING_JMP:
		{
			if (mnemonik_details->second.type != mnemonik_type::TYPE_JMP)
			{
				throw AsmException("Invalid syntax at operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
			}

			//addressing
			byte_2 = 0;
			auto addressing_mode_code_search = addressing_mode_hash_map.find(addressing_mode::REGISTER_DIRECT);
			byte_2 |= addressing_mode_code_search->second << 5;
			
			//reg code
			string register_number_str;
			if (operand.size() == 4) // *%r7
			{
				register_number_str = operand.substr(operand.size() - 1, 1);
			}
			else //*%r15 == psw
			{
				register_number_str = operand.substr(operand.size() - 2, 2);
			}
			int8_t register_number = stoi(register_number_str, NULL, 0); // register_number  = [0,7]
			byte_2 |= register_number << 1;

			instruction_code.push_back(byte_2);
			break;
		}
		case TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_JMP:
		{
			if (mnemonik_details->second.type != mnemonik_type::TYPE_JMP)
			{
				throw AsmException("Invalid syntax at operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
			}

			//addressing
			byte_2 = 0;
			auto addressing_mode_code_search = addressing_mode_hash_map.find(addressing_mode::REGISTER_INDIRECT);
			byte_2 |= addressing_mode_code_search->second << 5;
			//reg code
			string register_number_str;
			if (operand.size() == 6) //	*(%r7)
			{
				register_number_str = operand.substr(operand.size() - 2, 1);
			}
			else // *(%r15) == psw
			{
				register_number_str = operand.substr(operand.size() - 3, 2);
			}
			int8_t register_number = stoi(register_number_str, NULL, 0); // register_number  = [0,7]
			byte_2 |= register_number << 1;

			instruction_code.push_back(byte_2);

			break;
		}
		case TokenType::OPERAND_REGISTER_INDIRECT_ADDRESSING_WITH_OFFSET_JMP:
		{
			if (mnemonik_details->second.type != mnemonik_type::TYPE_JMP)
			{
				throw AsmException("Invalid syntax at operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
			}

			//addressing
			byte_2 = 0;
			auto addressing_mode_code_search = addressing_mode_hash_map.find(addressing_mode::REGISTER_INDIRECT_WITH_OFFSET);
			byte_2 |= addressing_mode_code_search->second << 5;
			//reg code
			//da li psw registar sme da se koristi za ovo ??????????????????????????????????
			size_t percentage_char_pos = operand.find('%', 0);
			string register_number_str;
			if (operand.size() - percentage_char_pos - 1 == 3) //*symbol(%r7) 11-7-1=3
			{
				register_number_str = operand.substr(percentage_char_pos + 2, 1);
			}
			else // *symbol(%r15) == psw
			{
				register_number_str = operand.substr(percentage_char_pos + 2, 2);
			}
			int8_t register_number = stoi(register_number_str, NULL, 0); // register_number  = [0,7]
			byte_2 |= register_number << 1;
			instruction_code.push_back(byte_2);

			//opernad value
			string offset_str = operand.substr(1, operand.size() - 1);
			size_t brackets_open_pos = offset_str.find('(', 0);
			offset_str = offset_str.substr(0, offset_str.size() - (offset_str.size() - brackets_open_pos));
			long offset = 0;
			Token offset_token = Token::parseToken(offset_str);
			switch (offset_token.getType())
			{
			case TokenType::DEC_NUMBER:
			{
				offset = stol(offset_str, NULL, 0);
				if (offset > 32767 || offset < -32767)
				{
					throw AsmException("Number '" + offset_str + "' at instruction '" + mnemonik + "' must be in range [-32767, 32767]. Error at line " + to_string(lineNo + 1));
				}
				break;
			}
			case TokenType::HEX_NUMBER:
			{
				offset = stol(offset_str, NULL, 0);
				if (offset > 65535)
				{
					throw AsmException("Number '" + offset_str + "' at instruction '" + mnemonik + "' must be in range [0x0000, 0xFFFF]. Error at line " + to_string(lineNo + 1));
				}
				break;
			}
			case TokenType::SYMBOL:
			{
				rel_type relocation_type = rel_type::R_16;
				if (register_number == 7) //offset = address(symbol) - PC = S(symbol) + offset(symbol) - * - 2;  * == address_of_this_offset_in_curr_section
				{
					if (SectionHeaderTable::getSymbolTable()->getSectionIndex(currSection_string) == SectionHeaderTable::getSymbolTable()->getIndexOfSectionWhereSymbolIsDefined(offset_str))
					{
						offset = (LC + 2 + 2); // == -PC  //no rel table
						offset *= -1;
					}
					else
					{
						offset -= 2;
						// for linker : S(symbol) - * ; S(symbol) == address of start of the section, * == LC + 2 (address of this offset)  - SYMBOL_LOCAL
						// for linker : address(symbol) - * ; addres(symbol) == address of the symbol, * == LC + 2 (address of this offset)  - SYMBOL_GLOBAL
					}
					relocation_type = rel_type::R_16_PC;
				}
				offset += SectionHeaderTable::getSymbolTable()->getSymbolValueAndCreateRelTableEntry(offset_str, static_cast<uint16_t>(LC + static_cast<uint16_t>(2)), currSection_string, relocation_type);
				break;
			}
			default:
				throw AsmException("Invalid syntax at operand '" + operand + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
				break;
			}
			int8_t data_low = offset & 0x00FF;
			int8_t data_high = (offset & 0xFF00) >> 8;
			instruction_code.push_back(data_low);
			instruction_code.push_back(data_high);

			break;
		}
		case TokenType::OPERAND_ABSOLUTE_DEC_JMP:
		case TokenType::OPERAND_ABSOLUTE_HEX_JMP:
		case TokenType::OPERAND_ABSOLUTE_SYMBOL_JMP:
		{
			//addressing
			byte_2 = 0;
			auto addressing_mode_code_search = addressing_mode_hash_map.find(addressing_mode::MEMORY_DIRECT);
			byte_2 |= addressing_mode_code_search->second << 5;
			instruction_code.push_back(byte_2);

			//operand value (address)
			long value;
			if (operand_token.getType() != TokenType::OPERAND_ABSOLUTE_SYMBOL_JMP)
			{
				value = stol(operand.substr(1, operand.size() - 1), NULL, 0);
				if (value > 65535 && operand_token.getType() == TokenType::HEX_NUMBER)
				{
					throw AsmException("Number '" + operand + "' at instruction '" + mnemonik + "' must be in range [0x0000, 0xFFFF]. Error at line " + to_string(lineNo + 1));
				}
				else if ((value > 65535 || value < 0) && operand_token.getType() == TokenType::DEC_NUMBER)
				{
					throw AsmException("Number '" + operand + "' at instruction '" + mnemonik + "' must be in range [0, 65535] (address). Error at line " + to_string(lineNo + 1));
				}
			}
			else
			{
				value = SectionHeaderTable::getSymbolTable()->getSymbolValueAndCreateRelTableEntry(operand.substr(1, operand.size() - 1), static_cast<uint16_t>(LC + static_cast<uint16_t>(2)), currSection_string, rel_type::R_16);
			}
			int8_t data_low = value & 0x00FF;
			int8_t data_high = (value & 0xFF00) >> 8;
			instruction_code.push_back(data_low);
			instruction_code.push_back(data_high);

			break;
		}
		default:
			throw AsmException("Unrecognized operand '" + operand_token.getValue() + "' at instruction '" + mnemonik + "'. Error at line " + to_string(lineNo + 1));
			break;
		}

	}

	//insert instruction_code in section
	for (int8_t code_byte : instruction_code)
	{
		SectionHeaderTable::getSectionTable(currSection_string)->insertByte(code_byte);
	}

	return static_cast<uint16_t>(instruction_code.size()); //return instruction size
}
