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
	Semicolon,			// ;
	LParen,				// (
	RParen,				// )
	LBrace,				// {
	RBrace,				// }
	Comma,				// ,

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
};

struct Token
{
	TokenType type;
	std::string value;
	Token(TokenType type, const std::string& value = "")
		: type(type), value(value)
	{
	}
	Token() : type(TokenType::Unknown)
	{
	}
};;