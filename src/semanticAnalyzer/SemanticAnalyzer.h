#pragma once
#include<vector>
#include"AST.h"
#include<unordered_map>

/*需求分析一下:
1.变量申明,变量使用
	
2.函数定义,函数申明,函数调用

3.二元运算是否类型相同

基本逻辑是一个栈,每进一个作用域就入栈一个符号表,然后出作用域就弹出
*/


class SemanticAnalyzer
{
private:
	// 符号表栈，每个符号表是一个unordered_map，key是名字，value是类型
	std::vector<std::unordered_map<std::string, std::string>> symbolTableStack; 
	// 是否含main函数
	bool hasMainFunction = false;
	bool hasError = false; // 记录是否发现语义错误，如果有错误就不继续分析了
public:
	bool analyze(ProgramAST* root); // 语义分析的入口函数
private:
	void enterScope(); // 进入一个新的作用域，压入一个新的符号表
	void exitScope(); // 退出当前作用域，弹出当前符号表

	std::string analyzeExpr(ExprAST* expr); // 分析表达式，返回表达式的类型
	void analyzeStmt(StmtAST* stmt); // 分析语句,语句分发

	// 其他辅助函数，例如处理变量声明、函数定义、类型检查等
	// 变量申明
	void analyzeVarDecl(VarDeclStmtAST* varDecl);
	// 变量使用
	std::string analyzeVariableExpr(VariableExprAST* varExpr);
	// 函数定义	
	void analyzeFunctionDecl(FunctionDeclAST* funcDecl);
	// if语句
	void analyzeIfStmt(IfStmtAST* ifStmt);
	// while语句
	void analyzeWhileStmt(WhileStmtAST* whileStmt);
	// do-while语句
	void analyzeDoWhileStmt(DoWhileStmtAST* doWhileStmt);
};