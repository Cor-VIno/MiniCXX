#include<iostream>
#include"Parser.h"
#include"LexerMap.h"

Parser::Parser(const std::vector<Token>& toks) : tokens(toks), pos(0)
{
}

const Token& Parser::peek() const
{
	if (pos < tokens.size())
		return tokens[pos];
	else
		return tokens.back();
}

const Token& Parser::peekNext() const
{
	if (pos < tokens.size() - 1)
		return tokens[pos + 1];
	else
		return tokens.back();
}
const Token& Parser::peekAhead(size_t n) const
{
	if (pos + n < tokens.size())
		return tokens[pos + n];
	else
		return tokens.back();
}

Token Parser::advance()
{
	if (pos < tokens.size())
		return tokens[pos++];
	else
		return tokens.back();
}

bool Parser::match(TokenType expectedType)
{
	if (peek().type == expectedType)
	{
		advance();
		return true;
	}
	return false;
}

Token Parser::consume(TokenType expectedType, const std::string& errorMessage)
{
	if (peek().type == expectedType)
	{
		return advance();
	}

	std::cerr << "Parser Error: " << errorMessage << " Got: " << peek().value << "\n";
	return Token();
}

// 解析原子
std::unique_ptr<ExprAST> Parser::parsePrimary()
{

	if (peek().type == TokenType::Number)
	{
		Token numToken = consume(TokenType::Number, "Expect number");
		return std::make_unique<NumberExprAST>(numToken.value);
	}

	else if (peek().type == TokenType::Identifier)
	{
		Token varToken = consume(TokenType::Identifier, "Expect variable name");
		if (peek().type != TokenType::LParen)
			return std::make_unique<VariableExprAST>(varToken.value);

		consume(TokenType::LParen, "Expect '(' after function name");

		std::vector<std::unique_ptr<ExprAST>> args;

		if (peek().type != TokenType::RParen)
		{
			while (true)
			{
				if (auto arg = parseExpression(0))
				{
					args.push_back(std::move(arg));
				}
				else
				{
					return nullptr;
				}

				if (peek().type == TokenType::Comma)
				{
					consume(TokenType::Comma, "Expect comma");
				}
				else
				{
					break;
				}
			}
		}


		consume(TokenType::RParen, "Expect ')' after arguments");

		return std::make_unique<CallExprAST>(varToken.value, std::move(args));
	}

	// 错误信息英语由ai生成
	else if (peek().type == TokenType::LParen)
	{
		consume(TokenType::LParen, "Expect '('");

		auto expr = parseExpression(0);
		if (!expr) return nullptr;

		consume(TokenType::RParen, "Expect ')' after expression");
		return expr;
	}

	else if (peek().type == TokenType::Keyword_string)
	{
		Token strToken = consume(TokenType::Keyword_string, "Expect string");
		return std::make_unique<StringExprAST>(strToken.value);
	}

	else
	{
		std::cerr << "Parser Error: Unexpected token '" << peek().value << "'\n";
		return nullptr;
	}
}

// 解析运算符
std::unique_ptr<ExprAST> Parser::parseExpression(int minPrec)
{
	auto lhs = parsePrimary();
	if (!lhs)
	{
		std::cerr << "Parser Error: Expected primary expression, got '" << peek().value << "'\n";
		return nullptr;
	}

	while (true)
	{
		int currentPrec = getTokenPrecedence();
		//运算优先级小或者根本不是运算符的时候退出
		if (currentPrec < minPrec)
		{
			return lhs;
		}

		Token opToken = advance();

		auto rhs = parseExpression(currentPrec + 1);
		if (!rhs)
		{
			std::cerr << "Parser Error: Expected expression after operator '" << opToken.value << "'\n";
			return nullptr;
		}

		lhs = std::make_unique<BinaryExprAST>(opToken.value, std::move(lhs), std::move(rhs));
	}
}

// 获取当前token的运算符优先级
int Parser::getTokenPrecedence()
{
	TokenType type = peek().type;

	auto it = PrecedenceMap.find(type);
	if (it != PrecedenceMap.end())
	{
		return it->second;
	}
	return -1;
}

// 总调用语句
std::unique_ptr<ProgramAST> Parser::parseProgram()
{
	std::vector<std::unique_ptr<StmtAST>> declarations;
	while (peek().type != TokenType::EndOfFile)
	{
		auto decl = parseStatement();
		if (decl)
		{
			declarations.push_back(std::move(decl));
		}
		else
		{
			std::cerr << "Parser Error: Failed to parse top-level declaration\n";
			return nullptr;
		}
	}
	return std::make_unique<ProgramAST>(std::move(declarations));
}



std::unique_ptr<StmtAST> Parser::parseStatement()
{
	//std::cout << "token.size:" << tokens.size() << "\n";

	//std::cout << pos << "\n";
	if (peek().type == TokenType::Keyword_if)
	{
		return parseIfStatement();
	}
	else if (peek().type == TokenType::Keyword_while)
	{
		return parseWhileStatement();
	}
	else if (peek().type == TokenType::Keyword_do)
	{
		return parseDoWhileStatement();
	}
	else if (peek().type == TokenType::Keyword_return)
	{
		return parseReturnStatement();
	}
	else if (peek().type == TokenType::Keyword_int ||
		peek().type == TokenType::Keyword_float ||
		peek().type == TokenType::Keyword_double ||
		peek().type == TokenType::Keyword_char ||
		peek().type == TokenType::Keyword_void)
	{
		if (peekAhead(2).type == TokenType::LParen)
		{
			return parseFunctionDecl();
		}
		else
		{
			return parseVarDecl();
		}
	}
	else if (peek().type == TokenType::LBrace)
	{
		return parseBlock();
	}
	else
	{
		return parseExpressionStmt();
	}


}

// 解析 "expr;"
std::unique_ptr<StmtAST> Parser::parseExpressionStmt()
{
	auto expr = parseExpression(0);
	if (!expr) return nullptr;
	consume(TokenType::Semicolon, "Expect ';' after expression");
	return std::make_unique<ExprStmtAST>(std::move(expr));
}

// 解析 "{ ... }"
std::unique_ptr<BlockStmtAST> Parser::parseBlock()
{
	consume(TokenType::LBrace, "Expect '{' to start block");
	std::vector<std::unique_ptr<StmtAST>> statements;
	while (peek().type != TokenType::RBrace && peek().type != TokenType::EndOfFile)
	{
		auto stmt = parseStatement();
		if (stmt)
		{
			statements.push_back(std::move(stmt));
		}
		else
		{
			std::cerr << "Parser Error: Failed to parse statement in block\n";
			return nullptr;
		}
	}
	consume(TokenType::RBrace, "Expect '}' to end block");
	return std::make_unique<BlockStmtAST>(std::move(statements));
}

// 解析 if
std::unique_ptr<StmtAST> Parser::parseIfStatement()
{
	consume(TokenType::Keyword_if, "Expect 'if'");
	consume(TokenType::LParen, "Expect '(' after 'if'");
	auto condition = parseExpression(0);
	if (!condition) return nullptr;
	consume(TokenType::RParen, "Expect ')' after condition");
	auto thenBranch = parseStatement();
	if (!thenBranch) return nullptr;
	std::unique_ptr<StmtAST> elseBranch = nullptr;
	if (match(TokenType::Keyword_else))
	{
		elseBranch = parseStatement();
		if (!elseBranch) return nullptr;
	}
	return std::make_unique<IfStmtAST>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

// 解析 while
std::unique_ptr<StmtAST> Parser::parseWhileStatement()
{
	consume(TokenType::Keyword_while, "Expect 'while'");
	consume(TokenType::LParen, "Expect '(' after 'while'");
	auto condition = parseExpression(0);
	if (!condition) return nullptr;
	consume(TokenType::RParen, "Expect ')' after condition");
	auto body = parseStatement();
	if (!body) return nullptr;
	return std::make_unique<WhileStmtAST>(std::move(condition), std::move(body));
}

// 解析 do-while
std::unique_ptr<StmtAST> Parser::parseDoWhileStatement()
{
	consume(TokenType::Keyword_do, "Expect 'do'");
	auto body = parseStatement();
	if (!body) return nullptr;
	consume(TokenType::Keyword_while, "Expect 'while' after 'do' block");
	consume(TokenType::LParen, "Expect '(' after 'while'");
	auto condition = parseExpression(0);
	if (!condition) return nullptr;
	consume(TokenType::RParen, "Expect ')' after condition");
	consume(TokenType::Semicolon, "Expect ';' after do-while statement");
	return std::make_unique<DoWhileStmtAST>(std::move(condition), std::move(body));
}

// 解析 return
std::unique_ptr<StmtAST> Parser::parseReturnStatement()
{
	consume(TokenType::Keyword_return, "Expect 'return'");
	auto value = parseExpression(0);
	if (!value) return nullptr;
	consume(TokenType::Semicolon, "Expect ';' after return statement");
	return std::make_unique<ReturnStmtAST>(std::move(value));
}

// 解析 "int a = ..."
std::unique_ptr<StmtAST> Parser::parseVarDecl()
{
	Token token = peek();
	//这如果要再次检测类型是否合法,就得在Token.h里把所有类型都列出来,然后在这里检测,感觉有点麻烦,所以就先不检测了,反正后面语义分析阶段还会检测的
	std::string varType = token.value;
	advance(); // 吃掉类型
	std::string varName = consume(TokenType::Identifier, "Expect variable name").value;
	/*std::cout << "peeked token: " << peek().value << "\n";
	if(peek().type == TokenType::Assign)*/
	std::unique_ptr<ExprAST> initValue = nullptr;
	if (match(TokenType::Assign))
	{
		initValue = parseExpression(0);
		if (!initValue) return nullptr;
	}
	consume(TokenType::Semicolon, "Expect ';' after variable declaration");
	return std::make_unique<VarDeclStmtAST>(varType, varName, std::move(initValue));
}

// 其他语句子类,例如函数声明、函数定义、类声明等,这些语句通常会包含一个名字、一个类型、一个参数列表和一个函数体等信息,它们在语义上是比较复杂的,所以我们需要单独的类来表示它们
// 
// 解析函数定义
// 暂且不支持类似void func(int a = 1){}的默认值的定义
std::unique_ptr<FunctionDeclAST> Parser::parseFunctionDecl()
{
	Token token = peek();
	std::string returnType = token.value;
	advance();
	std::string functionName = consume(TokenType::Identifier, "Expect function name").value;
	std::vector<std::pair<std::string, std::string>> parameters;
	consume(TokenType::LParen, "Expect '(' after function name");
	while (peek().type != TokenType::RParen)
	{
		Token token = peek();
		std::string paramType = token.value;
		advance();
		std::string paramName = consume(TokenType::Identifier, "Expect parameter name").value;
		parameters.emplace_back(paramType, paramName);
		if (peek().type == TokenType::Comma)
		{
			consume(TokenType::Comma, "Expect ',' between parameters");
		}
		else
		{
			break;
		}
	}
	consume(TokenType::RParen, "Expect ')' after parameters");
	std::unique_ptr<BlockStmtAST> body = parseBlock();
	if (!body) return nullptr;
	return std::make_unique<FunctionDeclAST>(returnType, functionName, parameters, std::move(body));
}

// 解析函数声明
std::unique_ptr<FunctionDeclStmtAST> Parser::parseFunctionDeclWithoutBody()
{
	Token token = peek();
	std::string returnType = token.value;
	advance();
	std::string functionName = consume(TokenType::Identifier, "Expect function name").value;
	std::vector<std::pair<std::string, std::string>> parameters;
	consume(TokenType::LParen, "Expect '(' after function name");
	while (peek().type != TokenType::RParen)
	{
		Token token = peek();
		std::string paramType = token.value;
		advance();
		std::string paramName = consume(TokenType::Identifier, "Expect parameter name").value;
		parameters.emplace_back(paramType, paramName);
		if (peek().type == TokenType::Comma)
		{
			consume(TokenType::Comma, "Expect ',' between parameters");
		}
		else
		{
			break;
		}
	}
	consume(TokenType::RParen, "Expect ')' after parameters");
	consume(TokenType::Semicolon, "Expect ';' after function declaration");
	return std::make_unique<FunctionDeclStmtAST>(returnType, functionName, parameters);
}