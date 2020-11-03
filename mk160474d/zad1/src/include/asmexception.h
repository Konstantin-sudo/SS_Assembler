#pragma once
#ifndef ASMEXCEPTION_H
#define ASMEXCEPTION_H

#include <string>
#include <exception>
using namespace std;

class AsmException: public exception{

public:

	AsmException(string _msg): msg(_msg) {}
	string getMsg() { return msg; }
	
private:

	string msg;
};

#endif
