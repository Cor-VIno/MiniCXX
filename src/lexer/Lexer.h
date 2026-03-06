#pragma once
#include<vector>
#include<string>
#include"Token.h"

//头文件只有声明，定义在.cpp文件中,防止重复包含
class Lexer
{
private:
	std::string source;
	size_t pos;
public:
	Lexer(const std::string& src);	// constructor
	Token getNextToken();			// get next token
	std::vector<Token> tokenize();	// tokenize entire source
private:
	char currentChar();			// cur char
	void advance();				// move to next char
	void skipWhitespace();		// skip spaces, tabs, newlines
	char peek();				// lookahead for next char
};
//后面英语太难了,就写中文了