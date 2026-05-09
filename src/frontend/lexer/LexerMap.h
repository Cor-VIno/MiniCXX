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
    {"do",     TokenType::Keyword_do},
    {"const",  TokenType::Keyword_const},
    {"break",  TokenType::Keyword_break},
    {"continue", TokenType::Keyword_continue}
};

inline const std::unordered_map<std::string, TokenType> SymbolsMap = {
    {"+",  TokenType::Plus},
    {"-",  TokenType::Minus},
    {"*",  TokenType::Star},
    {"/",  TokenType::Slash},
    {"%",  TokenType::Modulo},
    {"!",  TokenType::Not},
    {"&&", TokenType::LogicalAnd},
    {"||", TokenType::LogicalOr},
    {"&",  TokenType::BitwiseAnd},
    {"|",  TokenType::BitwiseOr},
    {"=",  TokenType::Assign},
    {"==", TokenType::Equal},
    {"!=", TokenType::NotEqual},
    {"<",  TokenType::Less},         
    {">",  TokenType::Greater},      
    {"<=", TokenType::LessEqual},
    {">=", TokenType::GreaterEqual},
    {";",  TokenType::Semicolon},
    {"(",  TokenType::LParen},
    {")",  TokenType::RParen},
    {"{",  TokenType::LBrace},
    {"}",  TokenType::RBrace},
    {"[",  TokenType::LBracket},
    {"]",  TokenType::RBracket},
    {",",  TokenType::Comma},
	{".",  TokenType::Period }
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
    {TokenType::Keyword_const,  "Keyword_const"},
    {TokenType::Keyword_break,  "Keyword_break"},
    {TokenType::Keyword_continue, "Keyword_continue"},

    // 标识符与字面量
    {TokenType::Identifier,     "Identifier"},
    {TokenType::Number,         "Number"},

    // 运算符与符号
    {TokenType::Plus,           "Plus"},
    {TokenType::Minus,          "Minus"},
    {TokenType::Star,           "Star"},
    {TokenType::Slash,    
    "Slash"},
    {TokenType::Modulo,         "Modulo"},
    {TokenType::Not,            "Not"},
    {TokenType::LogicalAnd,     "LogicalAnd"},
    {TokenType::LogicalOr,      "LogicalOr"},
    {TokenType::BitwiseAnd,     "BitwiseAnd"},
    {TokenType::BitwiseOr,      "BitwiseOr"},
    {TokenType::Assign,         "Assign"},
    {TokenType::Equal,          "Equal"},
    {TokenType::NotEqual,       "NotEqual"},
    {TokenType::Less,           "Less"},  
    {TokenType::Greater,        "Greater"},       
    {TokenType::LessEqual,      "LessEqual"},
    {TokenType::GreaterEqual,   "GreaterEqual"},
    {TokenType::Semicolon,      "Semicolon"},
    {TokenType::LParen,         "LParen"},
    {TokenType::RParen,         "RParen"},
    {TokenType::LBrace,         "LBrace"},
    {TokenType::RBrace,         "RBrace"},
    {TokenType::LBracket,       "LBracket"},
    {TokenType::RBracket,       "RBracket"},
    {TokenType::Comma,          "Comma"}, 
	{TokenType::Period,         "Period"},

    // 其他
    {TokenType::EndOfFile,      "EOF"},
    {TokenType::Unknown,        "Unknown"}
};

// 运算符优先级表 (数值越大，优先级越高)
inline const std::unordered_map<TokenType, int> PrecedenceMap = {
    // 逻辑或 ||
    {TokenType::LogicalOr,    1},
    // 逻辑与 &&
    {TokenType::LogicalAnd,   2},
    // 按位或 |
    {TokenType::BitwiseOr,    3},
    // 按位与 &
    {TokenType::BitwiseAnd,   4},

    // 关系运算符 (原来是 1，现在需要整体往上提)
    {TokenType::Equal,        5},
    {TokenType::NotEqual,     5},
    {TokenType::Less,         6},
    {TokenType::Greater,      6},
    {TokenType::LessEqual,    6},
    {TokenType::GreaterEqual, 6},

    // 加减
    {TokenType::Plus,         7},
    {TokenType::Minus,        7},

    // 乘除模
    {TokenType::Star,         8},
    {TokenType::Slash,        8},
    {TokenType::Modulo,       8}, // 模运算和乘除同级

    {TokenType::Period,       9}
};
