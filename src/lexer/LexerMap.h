#pragma once
#include "Token.h"
#include <unordered_map>
#include <string>

// cpp17才支持inline
inline const std::unordered_map<std::string, TokenType> KeywordsMap = {
    {"int",    TokenType::Keyword_int},
    {"float",  TokenType::Keyword_float},
    {"double", TokenType::Keyword_double},
    {"char",   TokenType::Keyword_char},
	{"string", TokenType::Keyword_string},
    {"bool",   TokenType::Keyword_bool},
    {"void",   TokenType::Keyword_void},

    {"return", TokenType::Keyword_return},
    {"if",     TokenType::Keyword_if},
    {"else",   TokenType::Keyword_else},
    {"for",    TokenType::Keyword_for},
    {"while",  TokenType::Keyword_while},
    {"do",     TokenType::Keyword_do}
};

inline const std::unordered_map<std::string, TokenType> SymbolsMap = {
    {"+",  TokenType::Plus},
    {"-",  TokenType::Minus},
    {"*",  TokenType::Star},
    {"/",  TokenType::Slash},
    {"=",  TokenType::Assign},
    {"==", TokenType::Equal},
    {"!=", TokenType::NotEqual},
    {"<",  TokenType::Less},         // 补上 Less
    {">",  TokenType::Greater},      // 补上 Greater
    {"<=", TokenType::LessEqual},
    {">=", TokenType::GreaterEqual},
    {";",  TokenType::Semicolon},
    {"(",  TokenType::LParen},
    {")",  TokenType::RParen},
    {"{",  TokenType::LBrace},
    {"}",  TokenType::RBrace},
    {",",  TokenType::Comma}
};

inline const std::unordered_map<TokenType, std::string> TokenNameMap = {
    // 关键字
    {TokenType::Keyword_int,    "Keyword_int"},
	{TokenType::Keyword_float,  "Keyword_float"},
	{TokenType::Keyword_double, "Keyword_double"},
	{TokenType::Keyword_char,   "Keyword_char"},
	{TokenType::Keyword_string, "Keyword_string"},
	{TokenType::Keyword_bool,   "Keyword_bool"},
	{TokenType::Keyword_void,   "Keyword_void"},

    {TokenType::Keyword_return, "Keyword_return"},
    {TokenType::Keyword_if,     "Keyword_if"},
    {TokenType::Keyword_else,   "Keyword_else"},
    {TokenType::Keyword_for,    "Keyword_for"},
    {TokenType::Keyword_while,  "Keyword_while"},
    {TokenType::Keyword_do,     "Keyword_do"},

    // 标识符与字面量
    {TokenType::Identifier,     "Identifier"},
    {TokenType::Number,         "Number"},

    // 运算符与符号
    {TokenType::Plus,           "Plus"},
    {TokenType::Minus,          "Minus"},
    {TokenType::Star,           "Star"},
    {TokenType::Slash,          "Slash"},
    {TokenType::Assign,         "Assign"},
    {TokenType::Equal,          "Equal"},
    {TokenType::NotEqual,       "NotEqual"},
    {TokenType::Less,           "Less"},           // 补上 Less
    {TokenType::Greater,        "Greater"},        // 补上 Greater
    {TokenType::LessEqual,      "LessEqual"},
    {TokenType::GreaterEqual,   "GreaterEqual"},
    {TokenType::Semicolon,      "Semicolon"},
    {TokenType::LParen,         "LParen"},
    {TokenType::RParen,         "RParen"},
    {TokenType::LBrace,         "LBrace"},
    {TokenType::RBrace,         "RBrace"},
    {TokenType::Comma,          "Comma"},          // 补上 Comma

    // 其他
    {TokenType::EndOfFile,      "EOF"},
    {TokenType::Unknown,        "Unknown"}
};

// 运算符优先级表 (数值越大，优先级越高)
inline const std::unordered_map<TokenType, int> PrecedenceMap = {
    {TokenType::Assign,       0},

    {TokenType::Equal,        1},
    {TokenType::NotEqual,     1},
    {TokenType::Less,         1},
    {TokenType::Greater,      1},
    {TokenType::LessEqual,    1},
    {TokenType::GreaterEqual, 1},

    {TokenType::Plus,         2},
    {TokenType::Minus,        2},

    {TokenType::Star,         3},
    {TokenType::Slash,        3}
};