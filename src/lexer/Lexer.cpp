#include <unordered_map>
#include "Lexer.h"
#include<cctype>
#include "LexerMap.h"

Lexer::Lexer(const std::string& src) : source(src), pos(0)
{
}

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

// peek函数用于查看下一个字符，但不移动pos指针，这对于处理双字符运算符非常有用
char Lexer::peek()
{
	if (pos + 1 < source.size())
		return source[pos + 1];
	else
		return '\0'; // EOF
}

Token Lexer::getNextToken()
{
	skipWhitespace();

	char ch = currentChar();

	// 在跳过空白符之后，解析标识符之前加入：
	if (ch == '"')
	{
		advance(); // 吃掉左边的双引号
		std::string strValue;
		while (currentChar() != '"' && currentChar() != '\0')
		{
			strValue += currentChar();
			advance();
		}
		advance(); // 吃掉右边的双引号
		return Token(TokenType::Keyword_string, strValue);
	}
	else if (ch == '/' && peek() == '*')
	{
		advance(); // 吃掉 '/'
		advance(); // 吃掉 '*'

		// 疯狂往后吃字符，直到遇见 '*/' 或文件结束
		while (currentChar() != '\0')
		{
			if (currentChar() == '*' && peek() == '/')
			{
				advance(); // 吃掉 '*'
				advance(); // 吃掉 '/'
				break;     // 成功跳出注释
			}
			advance();
		}

		return getNextToken(); // 递归调用自己，去拿注释后面的真正代码
	}
	else if (ch == '/' && peek() == '/')
	{
		advance(); // 吃掉 '/'
		advance(); // 吃掉 '/'
		// 吃掉直到行尾或文件结束
		while (currentChar() != '\0' && currentChar() != '\n')
		{
			advance();
		}
		return getNextToken(); // 递归调用自己，去拿注释后面的真正代码
	}


	if (std::isalpha(ch) || ch == '_') // identifier or keyword
	{
		std::string value;
		value += ch;
		advance();

		while (std::isalnum(currentChar()) || currentChar() == '_')
		{
			value += currentChar();
			advance();
		}

		auto it = KeywordsMap.find(value);
		if (it != KeywordsMap.end())
		{
			return Token(it->second, value);
		}

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
	else if (ch == '\0')	// EOF
	{
		return Token(TokenType::EndOfFile);
	}
	
	//注释
	//三字符运算符暂不支持
	//双字符运算符的优先级高于单字符运算符，所以先尝试匹配双字符运算符，如果匹配成功则返回对应的Token，否则再尝试匹配单字符运算符
	std::string doubleCharOp = std::string(1, ch) + peek();
	auto it2 = SymbolsMap.find(doubleCharOp);
	if (it2 != SymbolsMap.end())
	{
		advance();
		advance();
		return { it2->second, doubleCharOp };
	}

	std::string singleCharOp = std::string(1, ch);
	auto it1 = SymbolsMap.find(singleCharOp);
	if (it1 != SymbolsMap.end())
	{
		advance();
		return { it1->second, singleCharOp };
	}

	std::string value(1, ch);
	advance();
	return Token(TokenType::Unknown, value);
	
}

std::vector<Token> Lexer::tokenize()
{
	std::vector<Token> tokens;
	Token tok = getNextToken();

	while (tok.type != TokenType::EndOfFile)
	{
		tokens.push_back(tok);
		tok = getNextToken();
	}

	tokens.push_back(tok);

	return tokens;
}