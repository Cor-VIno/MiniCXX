#include"Lexer.h"
#include<cctype>

Lexer::Lexer(const std::string& src) : source(src), pos(0){}

char Lexer::currentChar()
{
	if (pos < source.size())
		return source[pos];
	else
		return '\0'; // EOF
}

void Lexer::advance()
{
	pos++;
}

void Lexer::skipWhitespace()
{
	while (std::isspace(currentChar()))
		advance();
}

Token Lexer::getNextToken()
{
	skipWhitespace();

	char ch = currentChar();

	if (std::isalpha(ch) || ch == '_') // identifier or keyword
	{
		std::string value;
		while (std::isalnum(currentChar()) || currentChar() == '_')
		{
			value += currentChar();
			advance();
		}
		if (value == "int")
			return Token(TokenType::Keyword_int, value);
		else if (value == "return")
			return Token(TokenType::Keyword_return, value);
		else
			return Token(TokenType::Identifier, value);
	}
	else if (std::isdigit(ch)) // number
	{
		std::string value;
		while (std::isdigit(currentChar()))
		{
			value += currentChar();
			advance();
		}
		return Token(TokenType::Number, value);
	}
	else if (ch == '=')
	{
		advance();
		return Token(TokenType::Assign, "=");
	}
	else if (ch == '+')
	{
		advance();
		return Token(TokenType::Plus, "+");
	}
	else if (ch == ';')
	{
		advance();
		return Token(TokenType::Semicolon, ";");
	}
	else if (ch == '(')
	{
		advance();
		return Token(TokenType::LParen, "(");
	}
	else if (ch == ')')
	{
		advance();
		return Token(TokenType::RParen, ")");
	}
	else if (ch == '{')
	{
		advance();
		return Token(TokenType::LBrance, "{");
	}
	else if (ch == '}')
	{
		advance();
		return Token(TokenType::RBrance, "}");
	}
	else if (ch == '\0')	// EOF
	{
		return Token(TokenType::EndOfFile);
	}
	else
	{
		std::string value(1, ch);
		advance();
		return Token(TokenType::Unknown, value);
	}
}