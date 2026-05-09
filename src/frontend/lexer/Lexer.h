#pragma once
#include <string>
#include <vector>
#include "Token.h"

class Lexer
{
private:
    std::string source;
    size_t pos;
    int line;
    int column;

public:
    explicit Lexer(const std::string& src);
    Token getNextToken();
    std::vector<Token> tokenize();

private:
    char currentChar();
    void advance();
    void skipWhitespace();
    char peek();
};
