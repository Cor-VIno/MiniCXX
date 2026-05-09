#pragma once

#include<string>

enum class TokenType
{
	Identifier,			// variable name, function name, etc.
	Number,				// integer literal
	EndOfFile,			// EOF
	Unknown,			// unrecognized token(error)

	Assign,				// =
	Equal,				// ==
	NotEqual,			// !=
	Less,				// <
	LessEqual,			// <=
	Greater,			// >
	GreaterEqual,		// >=
	Plus,				// +
	Minus,				// -
	Star,				// *
	Slash,				// /
	Modulo,         // %
	Not,            // !
	LogicalAnd,     // &&
	LogicalOr,      // ||
	BitwiseAnd,     // &  (按位与)
	BitwiseOr,      // |  (按位或)
	Semicolon,			// ;
	LParen,				// (
	RParen,				// )
	LBrace,				// {
	RBrace,				// }
	LBracket,           // [ 
	RBracket,           // ] 
	Comma,				// ,
	Period,				// .

	Keyword_int,
	Keyword_float,
	Keyword_double,
	Keyword_char,
	Keyword_string,
	Keyword_bool,
	Keyword_void,

	Keyword_return,
	Keyword_if,
	Keyword_else,
	Keyword_while,
	Keyword_for,
	Keyword_do,
	Keyword_const,
	Keyword_break,
	Keyword_continue,
};

struct Token
{
	TokenType type;
	std::string value;
	int line = 1;
	int column = 1;
	Token(TokenType type, const std::string& value = "", int line = 1, int column = 1)
		: type(type), value(value), line(line), column(column)
	{
	}
	Token() : type(TokenType::Unknown), line(1), column(1)
	{
	}
};;
