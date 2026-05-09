#pragma once
#include <vector>
#include <memory>
#include "frontend/lexer/Token.h"
#include"frontend/ast/AST.h"

class Parser
{
private:
    const std::vector<Token>& tokens;
    size_t pos;

    const Token& peek() const;
    const Token& peekNext() const;
    const Token& peekAhead(size_t n) const;
    Token advance();
    bool match(TokenType expectedType);
	Token consume(TokenType expectedType, const std::string& errorMessage);
	int getTokenPrecedence();
    bool isTypeToken(TokenType type) const;

    // 解析 "expr;"
    std::unique_ptr<StmtAST> parseExpressionStmt();  
    std::unique_ptr<StmtAST> parseAssignStmt(bool requireSemicolon = true);
    std::unique_ptr<LValueAST> parseLValue();
    bool isAssignmentStatement() const;
    // 解析 "{ ... }"
    std::unique_ptr<BlockStmtAST> parseBlock();
    // 解析 if
    std::unique_ptr<StmtAST> parseIfStatement();     
    // 解析 while
    std::unique_ptr<StmtAST> parseWhileStatement();  
    // 解析 do-while
    std::unique_ptr<StmtAST> parseDoWhileStatement();
    // 解析 for
    std::unique_ptr<StmtAST> parseForStatement();
    // 解析 return
    std::unique_ptr<StmtAST> parseReturnStatement(); 
    // 解析 "int a = ..."
    std::unique_ptr<StmtAST> parseVarDecl();         
    std::unique_ptr<FunctionParamAST> parseFunctionParam();
    // 解析函数定义
	std::unique_ptr<FunctionDeclAST> parseFunctionDecl();
    // 解析函数声明
	std::unique_ptr<FunctionDeclStmtAST> parseFunctionDeclWithoutBody();
    // 解析 "{1, 2, 3}" 这种初始化列表
    std::unique_ptr<ExprAST> parseInitListExpr();

    //解析一个模块
    std::unique_ptr<StmtAST> parseStatement();
    std::unique_ptr<ExprAST> parseUnary();
    std::unique_ptr<ExprAST> parsePrimary();
	std::unique_ptr<ExprAST> parseExpression(int minPrec);

public:
    Parser(const std::vector<Token>& toks);
	//总启动开关
    std::unique_ptr<ProgramAST> parseProgram();

};
