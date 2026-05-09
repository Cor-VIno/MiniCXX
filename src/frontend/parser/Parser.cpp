#include<iostream>
#include"frontend/parser/Parser.h"
#include"frontend/lexer/LexerMap.h"

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

bool Parser::isTypeToken(TokenType type) const
{
	return type == TokenType::Keyword_int
		|| type == TokenType::Keyword_float
		|| type == TokenType::Keyword_void;
}

std::unique_ptr<LValueAST> Parser::parseLValue()
{
	Token nameToken = consume(TokenType::Identifier, "Expect lvalue name");
	std::vector<std::unique_ptr<ExprAST>> indices;
	while (peek().type == TokenType::LBracket)
	{
		consume(TokenType::LBracket, "Expect '[' in lvalue");
		auto indexExpr = parseExpression(0);
		if (!indexExpr) return nullptr;
		indices.push_back(std::move(indexExpr));
		consume(TokenType::RBracket, "Expect ']' in lvalue");
	}
	if (!indices.empty())
	{
		return std::make_unique<ArrayAccessExprAST>(nameToken.value, std::move(indices));
	}
	return std::make_unique<VariableExprAST>(nameToken.value);
}

bool Parser::isAssignmentStatement() const
{
	if (peek().type != TokenType::Identifier)
	{
		return false;
	}

	size_t i = pos + 1;
	while (i < tokens.size() && tokens[i].type == TokenType::LBracket)
	{
		int depth = 1;
		++i;
		while (i < tokens.size() && depth > 0)
		{
			if (tokens[i].type == TokenType::LBracket) ++depth;
			else if (tokens[i].type == TokenType::RBracket) --depth;
			++i;
		}
	}
	return i < tokens.size() && tokens[i].type == TokenType::Assign;
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

		// 1. 如果后面跟着 '('，说明是函数调用 Func(x)
		if (peek().type == TokenType::LParen)
		{
			consume(TokenType::LParen, "Expect '(' after function name");
			std::vector<std::unique_ptr<ExprAST>> args;
			if (peek().type != TokenType::RParen)
			{
				while (true)
				{
					if (auto arg = parseExpression(0))
						args.push_back(std::move(arg));
					else
						return nullptr;

					if (peek().type == TokenType::Comma)
						consume(TokenType::Comma, "Expect comma");
					else
						break;
				}
			}
			consume(TokenType::RParen, "Expect ')' after arguments");
			return std::make_unique<CallExprAST>(varToken.value, std::move(args), varToken.line);
		}
		// 2. 如果后面跟着 '['，说明是数组访问 arr[i][j]！(新增逻辑)
		else if (peek().type == TokenType::LBracket)
		{
			std::vector<std::unique_ptr<ExprAST>> indices;
			// 疯狂吃掉所有的 [xxx]
			while (peek().type == TokenType::LBracket)
			{
				consume(TokenType::LBracket, "Expect '['");
				auto indexExpr = parseExpression(0);
				if (!indexExpr) return nullptr;
				indices.push_back(std::move(indexExpr));
				consume(TokenType::RBracket, "Expect ']'");
			}
			return std::make_unique<ArrayAccessExprAST>(varToken.value, std::move(indices));
		}
		// 3. 啥都没跟，说明只是个普通的变量名 x
		else
		{
			return std::make_unique<VariableExprAST>(varToken.value);
		}
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
		Token strToken = consume(TokenType::Keyword_string, "Expect string literal");
		return std::make_unique<StringExprAST>(strToken.value);
	}

	else
	{
		std::cerr << "Parser Error: Unexpected token '" << peek().value << "'\n";
		return nullptr;
	}
}

std::unique_ptr<ExprAST> Parser::parseUnary()
{
	if (peek().type == TokenType::Plus || peek().type == TokenType::Minus || peek().type == TokenType::Not)
	{
		Token opToken = advance();
		auto operand = parseUnary();
		if (!operand) return nullptr;
		return std::make_unique<UnaryExprAST>(opToken.value, std::move(operand));
	}
	return parsePrimary();
}

// 解析运算符
std::unique_ptr<ExprAST> Parser::parseExpression(int minPrec)
{
	auto lhs = parseUnary();
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
	if (peek().type == TokenType::Semicolon)
	{
		consume(TokenType::Semicolon, "Expect ';'");
		return std::make_unique<EmptyStmtAST>();
	}
	else if (peek().type == TokenType::Keyword_if)
	{
		return parseIfStatement();
	}
	else if (peek().type == TokenType::Keyword_while)
	{
		return parseWhileStatement();
	}
	else if (peek().type == TokenType::Keyword_for)
	{
		std::cerr << "Parser Error: 'for' is not part of SysY; use while instead\n";
		return nullptr;
	}
	else if (peek().type == TokenType::Keyword_do)
	{
		std::cerr << "Parser Error: 'do-while' is not part of SysY; use while instead\n";
		return nullptr;
	}
	else if (peek().type == TokenType::Keyword_return)
	{
		return parseReturnStatement();
	}
	else if (peek().type == TokenType::Keyword_break)
	{
		consume(TokenType::Keyword_break, "Expect 'break'");
		consume(TokenType::Semicolon, "Expect ';' after break");
		return std::make_unique<BreakStmtAST>();
	}
	else if (peek().type == TokenType::Keyword_continue)
	{
		consume(TokenType::Keyword_continue, "Expect 'continue'");
		consume(TokenType::Semicolon, "Expect ';' after continue");
		return std::make_unique<ContinueStmtAST>();
	}
	else if (isAssignmentStatement())
	{
		return parseAssignStmt();
	}
	else if (peek().type == TokenType::Keyword_const || isTypeToken(peek().type))
	{
		size_t offset = peek().type == TokenType::Keyword_const ? 1 : 0;
		if (peekAhead(offset + 2).type == TokenType::LParen)
		{
			size_t i = pos + offset + 2;
			int depth = 0;
			while (i < tokens.size())
			{
				if (tokens[i].type == TokenType::LParen) ++depth;
				else if (tokens[i].type == TokenType::RParen)
				{
					--depth;
					if (depth == 0)
					{
						break;
					}
				}
				++i;
			}
			if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::Semicolon)
			{
				return parseFunctionDeclWithoutBody();
			}
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

std::unique_ptr<StmtAST> Parser::parseAssignStmt(bool requireSemicolon)
{
	auto target = parseLValue();
	if (!target) return nullptr;
	consume(TokenType::Assign, "Expect '=' in assignment statement");
	auto value = parseExpression(0);
	if (!value) return nullptr;
	if (requireSemicolon)
	{
		consume(TokenType::Semicolon, "Expect ';' after assignment statement");
	}
	return std::make_unique<AssignStmtAST>(std::move(target), std::move(value));
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

// 解析 for 语句
std::unique_ptr<StmtAST> Parser::parseForStatement()
{
	consume(TokenType::Keyword_for, "Expect 'for'");
	consume(TokenType::LParen, "Expect '(' after 'for'");

	// 1. 解析 init (初始化)
	std::unique_ptr<StmtAST> init = nullptr;
	if (match(TokenType::Semicolon))
	{
		// 场景: for (; ...)  无初始化
	}
	else if (peek().type == TokenType::Keyword_const || peek().type == TokenType::Keyword_int || peek().type == TokenType::Keyword_float)
	{
		// 场景: for (int i = 0; ...)
		// 注意：你的 parseVarDecl() 里面已经调用了 consume(Semicolon)，所以这里不用再吃分号了
		init = parseVarDecl();
	}
	else
	{
		if (isAssignmentStatement())
		{
			init = parseAssignStmt();
		}
		else
		{
			init = parseExpressionStmt();
		}
	}

	// 2. 解析 condition (条件)
	std::unique_ptr<ExprAST> condition = nullptr;
	if (!match(TokenType::Semicolon))
	{
		condition = parseExpression(0);
		consume(TokenType::Semicolon, "Expect ';' after loop condition");
	}

	// 3. 解析 increment (步进)
	std::unique_ptr<StmtAST> increment = nullptr;
	if (peek().type != TokenType::RParen)
	{
		if (isAssignmentStatement())
		{
			increment = parseAssignStmt(false);
		}
		else
		{
			auto expr = parseExpression(0);
			if (!expr) return nullptr;
			increment = std::make_unique<ExprStmtAST>(std::move(expr));
		}
	}
	consume(TokenType::RParen, "Expect ')' after for clauses");

	// 4. 解析 body (循环体)
	auto body = parseStatement();
	if (!body) return nullptr;

	return std::make_unique<ForStmtAST>(std::move(init), std::move(condition), std::move(increment), std::move(body));
}

// 解析 return
std::unique_ptr<StmtAST> Parser::parseReturnStatement()
{
	consume(TokenType::Keyword_return, "Expect 'return'");
	std::unique_ptr<ExprAST> value = nullptr;
	if (peek().type != TokenType::Semicolon)
	{
		value = parseExpression(0);
		if (!value) return nullptr;
	}
	consume(TokenType::Semicolon, "Expect ';' after return statement");
	return std::make_unique<ReturnStmtAST>(std::move(value));
}

// 解析变量或数组声明
std::unique_ptr<StmtAST> Parser::parseVarDecl()
{
	bool isConst = match(TokenType::Keyword_const);
	Token token = peek();
	std::string varType = token.value;
	advance(); // 吃掉类型 (int/float)
	std::vector<std::unique_ptr<StmtAST>> declarations;

	while (true)
	{
		std::string varName = consume(TokenType::Identifier, "Expect variable name").value;
		std::vector<std::unique_ptr<ExprAST>> dimensions;

		while (peek().type == TokenType::LBracket)
		{
			consume(TokenType::LBracket, "Expect '['");
			dimensions.push_back(parseExpression(0));
			consume(TokenType::RBracket, "Expect ']'");
		}

		std::unique_ptr<ExprAST> initValue = nullptr;
		if (match(TokenType::Assign))
		{
			if (peek().type == TokenType::LBrace)
			{
				initValue = parseInitListExpr();
			}
			else
			{
				initValue = parseExpression(0);
			}
		}
		else if (isConst)
		{
			std::cerr << "Parser Error: const declaration must have initializer\n";
			return nullptr;
		}

		if (!dimensions.empty())
		{
			declarations.push_back(std::make_unique<ArrayDeclAST>(varType, varName, std::move(dimensions), std::move(initValue), isConst));
		}
		else
		{
			declarations.push_back(std::make_unique<VarDeclStmtAST>(varType, varName, std::move(initValue), isConst));
		}

		if (!match(TokenType::Comma))
		{
			break;
		}
	}

	consume(TokenType::Semicolon, "Expect ';' after declaration");
	if (declarations.size() == 1)
	{
		return std::move(declarations.front());
	}
	return std::make_unique<DeclGroupAST>(std::move(declarations));
}
// 递归解析嵌套的初始化列表：{1, 2, {3, 4}}
std::unique_ptr<ExprAST> Parser::parseInitListExpr()
{
	consume(TokenType::LBrace, "Expect '{' to start init list");
	std::vector<std::unique_ptr<ExprAST>> elements;

	if (peek().type != TokenType::RBrace)
	{
		while (true)
		{
			// 如果遇到 '{'，说明是多维数组的嵌套大括号，递归调用自己！
			if (peek().type == TokenType::LBrace)
			{
				elements.push_back(parseInitListExpr());
			}
			// 否则就是一个普通的表达式（比如数字）
			else
			{
				elements.push_back(parseExpression(0));
			}

			if (peek().type == TokenType::Comma)
			{
				consume(TokenType::Comma, "Expect ',' in init list");
			}
			else
			{
				break;
			}
		}
	}

	consume(TokenType::RBrace, "Expect '}' to end init list");
	return std::make_unique<InitListExprAST>(std::move(elements));
}

std::unique_ptr<FunctionParamAST> Parser::parseFunctionParam()
{
	if (!isTypeToken(peek().type) || peek().type == TokenType::Keyword_void)
	{
		std::cerr << "Parser Error: Expect function parameter type\n";
		return nullptr;
	}

	std::string paramType = advance().value;
	std::string paramName = consume(TokenType::Identifier, "Expect parameter name").value;
	bool isArray = false;
	std::vector<std::unique_ptr<ExprAST>> dimensions;

	if (match(TokenType::LBracket))
	{
		isArray = true;
		consume(TokenType::RBracket, "Expect ']' after first array parameter dimension");
		while (match(TokenType::LBracket))
		{
			auto dim = parseExpression(0);
			if (!dim) return nullptr;
			dimensions.push_back(std::move(dim));
			consume(TokenType::RBracket, "Expect ']' after array parameter dimension");
		}
	}

	return std::make_unique<FunctionParamAST>(paramType, paramName, isArray, std::move(dimensions));
}

// 其他语句子类,例如函数声明、函数定义、类声明等,这些语句通常会包含一个名字、一个类型、一个参数列表和一个函数体等信息,它们在语义上是比较复杂的,所以我们需要单独的类来表示它们
// 
// 解析函数定义
// 暂且不支持类似void func(int a = 1){}的默认值的定义
std::unique_ptr<FunctionDeclAST> Parser::parseFunctionDecl()
{
	if (match(TokenType::Keyword_const))
	{
		std::cerr << "Parser Error: Function return type cannot be const\n";
		return nullptr;
	}
	Token token = peek();
	std::string returnType = token.value;
	advance();
	std::string functionName = consume(TokenType::Identifier, "Expect function name").value;
	std::vector<FunctionParamAST> parameters;
	consume(TokenType::LParen, "Expect '(' after function name");
	if (peek().type == TokenType::Keyword_void && peekNext().type == TokenType::RParen)
	{
		advance();
	}
	while (peek().type != TokenType::RParen)
	{
		auto param = parseFunctionParam();
		if (!param) return nullptr;
		parameters.push_back(std::move(*param));
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
	return std::make_unique<FunctionDeclAST>(returnType, functionName, std::move(parameters), std::move(body));
}

// 解析函数声明
std::unique_ptr<FunctionDeclStmtAST> Parser::parseFunctionDeclWithoutBody()
{
	if (match(TokenType::Keyword_const))
	{
		std::cerr << "Parser Error: Function return type cannot be const\n";
		return nullptr;
	}
	Token token = peek();
	std::string returnType = token.value;
	advance();
	std::string functionName = consume(TokenType::Identifier, "Expect function name").value;
	std::vector<FunctionParamAST> parameters;
	consume(TokenType::LParen, "Expect '(' after function name");
	if (peek().type == TokenType::Keyword_void && peekNext().type == TokenType::RParen)
	{
		advance();
	}
	while (peek().type != TokenType::RParen)
	{
		auto param = parseFunctionParam();
		if (!param) return nullptr;
		parameters.push_back(std::move(*param));
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
	return std::make_unique<FunctionDeclStmtAST>(returnType, functionName, std::move(parameters));
}

