#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Token.h"
#include <iostream>
// ASTNode 是所有 AST 节点的基类，提供了一个统一的接口和类型层次结构
class ASTNode
{
public:
	virtual ~ASTNode() = default;

	virtual void print(int indent = 0) const = 0;
};
//======================================================================

// 表达式基类,表达式是程序中的一个计算单元，表示一个值的计算过程，例如数字字面量、变量、二元运算等。每个表达式都可以包含一个或多个子表达式，形成一个树状结构来表示复杂的计算过程。
class ExprAST : public ASTNode
{
public:
	virtual ~ExprAST() = default;
};

// 语句基类,语句是程序的基本执行单元，表示程序中的一个操作或行为，例如表达式语句、条件语句、循环语句等。每个语句都可以包含一个或多个表达式，或者其他语句，形成一个树状结构来表示程序的整体逻辑。
class StmtAST : public ASTNode
{
public:
	virtual ~StmtAST() = default;
};

// 代表整个程序/文件
class ProgramAST : public ASTNode
{
public:
	std::vector<std::unique_ptr<StmtAST>> declarations;

	ProgramAST(std::vector<std::unique_ptr<StmtAST>> declarations)
		: declarations(std::move(declarations))
	{
	}

	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "Program:\n";
		for (const auto& decl : declarations)
		{
			if (decl) decl->print(indent + 2);
		}
	}
};
//======================================================================
// 表达式子类
// NumberExprAST 表示一个数字字面量，例如 42
class NumberExprAST : public ExprAST
{
public:
	std::string value;
	NumberExprAST(const std::string& val) : value(val)
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "NumberExpr: " << value << "\n";
	}
};

// VariableExprAST 表示一个变量，例如 x
class VariableExprAST : public ExprAST
{
public:
	std::string name;
	VariableExprAST(const std::string& name) : name(name)
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "VariableExpr: " << name << "\n";
	}
};

// BinaryExprAST 表示一个二元运算，例如 a + b
class BinaryExprAST : public ExprAST
{
public:
	std::string op;
	std::unique_ptr<ExprAST> LHS, RHS;
	// op 是运算符，例如 "+"，LHS 是左操作数，RHS 是右操作数
	// move是为了转移所有权，防止资源泄漏
	BinaryExprAST(const std::string& op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
		: op(op), LHS(std::move(lhs)), RHS(std::move(rhs))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "BinaryExpr: " << op << "\n";
		if (LHS) LHS->print(indent + 2);
		if (RHS) RHS->print(indent + 2);
	}
};

class StringExprAST : public ExprAST
{
public:
	std::string value;
	StringExprAST(const std::string& val) : value(val)
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "StringExpr: \"" << value << "\"\n";
	}
};

//======================================================================
// 语句子类
// ExprStmtAST 表示一个表达式语句，例如 a + b;
class ExprStmtAST : public StmtAST
{
public:
	std::unique_ptr<ExprAST> expr;
	ExprStmtAST(std::unique_ptr<ExprAST> expr) : expr(std::move(expr))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "ExprStmt:\n";
		if (expr) expr->print(indent + 2);
	}
};

// BlockStmtAST 表示一个代码块，例如 { ... }
class BlockStmtAST : public StmtAST
{
public:
	std::vector<std::unique_ptr<StmtAST>> statements;
	// 错误代码,构造里面不能直接用成员初始化列表来初始化vector，vector复制会触发内部元素的复制,而vector的元素是unique_ptr，不能直接复制
	//BlockStmtAST(const std::vector<std::unique_ptr<StmtAST>>& Statements) : statements(std::move(Statements)){}
	// 而下面的代码是正确的，使用std::move来转移所有权，防止资源泄漏,因为std::move会将Statements转换为右值引用，允许vector的移动构造函数被调用，从而正确地转移内部元素的所有权，而不是尝试复制它们。
	BlockStmtAST(std::vector<std::unique_ptr<StmtAST>> Statements) : statements(std::move(Statements))
	{
	}

	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "BlockStmt:\n";
		for (const auto& stmt : statements)
		{
			if (stmt) stmt->print(indent + 2);
		}
	}
};

// IfStmtAST 表示一个 if 语句，例如 if (condition) thenBranch else elseBranch
class IfStmtAST : public StmtAST
{
public:
	std::unique_ptr<ExprAST> condition;
	// 因为可能会是不带{}的语句
	std::unique_ptr<StmtAST> thenBranch;
	// elseBranch是可选的，所以用unique_ptr来表示，如果没有else分支，则elseBranch为nullptr
	// 为什么不分开来,将else单独写成ElseStmtAST,然后再来一个elseif?
	// 非常简单
	// 1. 语法上，if-else语句的结构是固定的，else分支是if语句的一部分，而不是一个独立的语句，因此将else分支单独写成ElseStmtAST会导致语法结构不清晰。
	// 2. 语义上，else分支的执行是依赖于if条件的结果的，如果将else分开成一个独立的语句，那么在语义分析和代码生成阶段就需要额外的逻辑来处理这种依赖关系，增加了实现的复杂度。
	// 3. else后面语句可以是单个语句,而if(){}也是单个语句,只不过是写到一行去了
	std::unique_ptr<StmtAST> elseBranch;
	IfStmtAST(std::unique_ptr<ExprAST> condition, std::unique_ptr<StmtAST> thenBranch, std::unique_ptr<StmtAST> elseBranch = nullptr)
		: condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "IfStmt:\n";
		if (condition) condition->print(indent + 2);
		if (thenBranch) thenBranch->print(indent + 2);
		if (elseBranch)
		{
			std::cout << indentStr << "ElseBranch:\n";
			elseBranch->print(indent + 2);
		}
	}
};

// WhileStmtAST 表示一个 while 语句，例如 while (condition) body
class WhileStmtAST : public StmtAST
{
public:
	std::unique_ptr<ExprAST> condition;
	std::unique_ptr<StmtAST> body;
	WhileStmtAST(std::unique_ptr<ExprAST> condition, std::unique_ptr<StmtAST> body)
		: condition(std::move(condition)), body(std::move(body))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "WhileStmt:\n";
		if (condition) condition->print(indent + 2);
		if (body) body->print(indent + 2);
	}
};

// DoWhileStmtAST 表示一个 do-while 语句，例如 do body while (condition)
class DoWhileStmtAST : public StmtAST
{
public:
	std::unique_ptr<ExprAST> condition;
	std::unique_ptr<StmtAST> body;
	DoWhileStmtAST(std::unique_ptr<ExprAST> condition, std::unique_ptr<StmtAST> body)
		: condition(std::move(condition)), body(std::move(body))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "DoWhileStmt:\n";
		if (body) body->print(indent + 2);
		if (condition)
		{
			std::cout << indentStr << "Condition:\n";
			condition->print(indent + 2);
		}
	}
};

// ReturnStmtAST 表示一个 return 语句，例如 return expr;
class ReturnStmtAST : public StmtAST
{
public:
	std::unique_ptr<ExprAST> returnValue;
	ReturnStmtAST(std::unique_ptr<ExprAST> returnValue) : returnValue(std::move(returnValue))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "ReturnStmt:\n";
		if (returnValue) returnValue->print(indent + 2);
	}
};

// VarDeclStmtAST 表示一个变量声明语句，例如 int x = 10;
class VarDeclStmtAST : public StmtAST
{
public:
	std::string varType;
	std::string varName;	// 这个并非等同于VariableExprAST,因为一个是变量声明,另一个是变量使用,虽然它们都包含一个名字,但它们在语义上是不同的,所以我们需要两个不同的类来表示它们
	std::unique_ptr<ExprAST> initValue; // 可选的初始化表达式
	VarDeclStmtAST(const std::string& varType, const std::string& varName, std::unique_ptr<ExprAST> initValue = nullptr)
		: varType(varType), varName(varName), initValue(std::move(initValue))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "VarDeclStmt: " << varType << " " << varName << "\n";
		if (initValue)
		{
			std::cout << indentStr << "Initializer:\n";
			initValue->print(indent + 2);
		}
	}
};
//======================================================================
// 其他语句子类,例如函数声明、函数定义、类声明等,这些语句通常会包含一个名字、一个类型、一个参数列表和一个函数体等信息,它们在语义上是比较复杂的,所以我们需要单独的类来表示它们

// 函数定义
class FunctionDeclAST : public StmtAST
{
public:
	std::string returnType;
	std::string functionName;
	std::vector<std::pair<std::string, std::string>> parameters; // 参数列表，包含参数类型和参数名
	std::unique_ptr<BlockStmtAST> body; // 函数体
	FunctionDeclAST(const std::string& returnType, const std::string& functionName,
		const std::vector<std::pair<std::string, std::string>>& parameters,
		std::unique_ptr<BlockStmtAST> body)
		: returnType(returnType), functionName(functionName), parameters(parameters), body(std::move(body))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "FunctionDecl: " << returnType << " " << functionName << "(";
		for (size_t i = 0; i < parameters.size(); ++i)
		{
			std::cout << parameters[i].first << " " << parameters[i].second;
			if (i < parameters.size() - 1)
				std::cout << ", ";
		}
		std::cout << ")\n";
		if (body) body->print(indent + 2);
	}
};

// 函数声明
class FunctionDeclStmtAST : public StmtAST
{
public:
	std::string returnType;
	std::string functionName;
	std::vector<std::pair<std::string, std::string>> parameters;
	FunctionDeclStmtAST(const std::string& returnType, const std::string& functionName,
		const std::vector<std::pair<std::string, std::string>>& parameters)
		: returnType(returnType), functionName(functionName), parameters(parameters)
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "FunctionDeclStmt: " << returnType << " " << functionName << "(";
		for (size_t i = 0; i < parameters.size(); ++i)
		{
			std::cout << parameters[i].first << " " << parameters[i].second;
			if (i < parameters.size() - 1)
				std::cout << ", ";
		}
		std::cout << ")\n";
	}
};

// CallExprAST表示函数调用
class CallExprAST : public ExprAST
{
public:
	std::string name;
	std::vector<std::unique_ptr<ExprAST>> args;
	CallExprAST(const std::string& name, std::vector<std::unique_ptr<ExprAST>> args)
		: name(name), args(std::move(args))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "CallExpr: " << name << "\n";
		for (const auto& arg : args)
		{
			if (arg) arg->print(indent + 2);
		}
	}
};