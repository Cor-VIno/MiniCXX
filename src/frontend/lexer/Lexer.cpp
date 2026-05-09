#include "Lexer.h"
#include "LexerMap.h"

#include <cctype>
#include <unordered_map>

Lexer::Lexer(const std::string& src) : source(src), pos(0), line(1), column(1)
{
}

char Lexer::currentChar()
{
    if (pos < source.size())
        return source[pos];
    return '\0';
}

void Lexer::advance()
{
    if (currentChar() == '\n')
    {
        ++line;
        column = 1;
    }
    else
    {
        ++column;
    }
    pos++;
}

void Lexer::skipWhitespace()
{
    while (std::isspace(static_cast<unsigned char>(currentChar())))
        advance();
}

char Lexer::peek()
{
    if (pos + 1 < source.size())
        return source[pos + 1];
    return '\0';
}

Token Lexer::getNextToken()
{
    skipWhitespace();

    char ch = currentChar();
    int tokenLine = line;
    int tokenColumn = column;

    if (ch == '"')
    {
        advance();
        std::string strValue;
        while (currentChar() != '"' && currentChar() != '\0')
        {
            if (currentChar() == '\\')
            {
                strValue += currentChar();
                advance();
                if (currentChar() != '\0')
                {
                    strValue += currentChar();
                    advance();
                }
                continue;
            }
            strValue += currentChar();
            advance();
        }
        if (currentChar() == '"') advance();
        return Token(TokenType::Keyword_string, strValue, tokenLine, tokenColumn);
    }

    if (ch == '/' && peek() == '*')
    {
        advance();
        advance();
        while (currentChar() != '\0')
        {
            if (currentChar() == '*' && peek() == '/')
            {
                advance();
                advance();
                break;
            }
            advance();
        }
        return getNextToken();
    }

    if (ch == '/' && peek() == '/')
    {
        advance();
        advance();
        while (currentChar() != '\0' && currentChar() != '\n')
            advance();
        return getNextToken();
    }

    if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_')
    {
        std::string value;
        value += ch;
        advance();

        while (std::isalnum(static_cast<unsigned char>(currentChar())) || currentChar() == '_')
        {
            value += currentChar();
            advance();
        }

        auto it = KeywordsMap.find(value);
        if (it != KeywordsMap.end())
            return Token(it->second, value, tokenLine, tokenColumn);

        return Token(TokenType::Identifier, value, tokenLine, tokenColumn);
    }

    if (std::isdigit(static_cast<unsigned char>(ch)) || (ch == '.' && std::isdigit(static_cast<unsigned char>(peek()))))
    {
        std::string value;
        bool isHex = false;
        bool hasDot = false;

        if (currentChar() == '0' && (peek() == 'x' || peek() == 'X'))
        {
            value += currentChar();
            advance();
            value += currentChar();
            advance();
            isHex = true;
            while (std::isxdigit(static_cast<unsigned char>(currentChar())) || currentChar() == '.')
            {
                if (currentChar() == '.')
                {
                    if (hasDot) break;
                    hasDot = true;
                }
                value += currentChar();
                advance();
            }
        }
        else
        {
            while (std::isdigit(static_cast<unsigned char>(currentChar())) || currentChar() == '.')
            {
                if (currentChar() == '.')
                {
                    if (hasDot) break;
                    hasDot = true;
                }
                value += currentChar();
                advance();
            }
        }

        if ((!isHex && (currentChar() == 'e' || currentChar() == 'E'))
            || (isHex && (currentChar() == 'p' || currentChar() == 'P')))
        {
            value += currentChar();
            advance();
            if (currentChar() == '+' || currentChar() == '-')
            {
                value += currentChar();
                advance();
            }
            while (std::isdigit(static_cast<unsigned char>(currentChar())))
            {
                value += currentChar();
                advance();
            }
        }

        if (currentChar() == 'f' || currentChar() == 'F')
        {
            value += currentChar();
            advance();
        }

        return Token(TokenType::Number, value, tokenLine, tokenColumn);
    }

    if (ch == '\0')
        return Token(TokenType::EndOfFile, "", tokenLine, tokenColumn);

    std::string doubleCharOp = std::string(1, ch) + peek();
    auto it2 = SymbolsMap.find(doubleCharOp);
    if (it2 != SymbolsMap.end())
    {
        advance();
        advance();
        return { it2->second, doubleCharOp, tokenLine, tokenColumn };
    }

    std::string singleCharOp = std::string(1, ch);
    auto it1 = SymbolsMap.find(singleCharOp);
    if (it1 != SymbolsMap.end())
    {
        advance();
        return { it1->second, singleCharOp, tokenLine, tokenColumn };
    }

    std::string value(1, ch);
    advance();
    return Token(TokenType::Unknown, value, tokenLine, tokenColumn);
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
