#pragma once
#include <string>
#include <memory>
#include <vector>
#include "frontend\lexer\Token.h"
#include <iostream>

class ASTVisitor;
class Value;

// ASTNode 是所有 AST 节点的基类，提供了一个统一的接口和类型层次结构
class ASTNode
{
public:
	virtual ~ASTNode() = default;

	virtual void print(int indent = 0) const = 0;
	virtual Value* accept(ASTVisitor& visitor) = 0;
};
//======================================================================

// 表达式基类,表达式是程序中的一个计算单元，表示一个值的计算过程，例如数字字面量、变量、二元运算等。每个表达式都可以包含一个或多个子表达式，形成一个树状结构来表示复杂的计算过程。
class ExprAST : public ASTNode
{
public:
	virtual ~ExprAST() = default;
};

class LValueAST : public ExprAST
{
public:
	virtual ~LValueAST() = default;
};

struct FunctionParamAST
{
	std::string type;
	std::string name;
	bool isArray = false;
	std::vector<std::unique_ptr<ExprAST>> dimensions;

	FunctionParamAST() = default;
	FunctionParamAST(const std::string& type, const std::string& name)
		: type(type), name(name)
	{
	}
	FunctionParamAST(const std::string& type, const std::string& name, bool isArray,
		std::vector<std::unique_ptr<ExprAST>> dimensions)
		: type(type), name(name), isArray(isArray), dimensions(std::move(dimensions))
	{
	}
	FunctionParamAST(FunctionParamAST&&) noexcept = default;
	FunctionParamAST& operator=(FunctionParamAST&&) noexcept = default;
	FunctionParamAST(const FunctionParamAST&) = delete;
	FunctionParamAST& operator=(const FunctionParamAST&) = delete;
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

	Value* accept(ASTVisitor& visitor) override;
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
	Value* accept(ASTVisitor& visitor) override;
};

// VariableExprAST 表示一个变量，例如 x
class VariableExprAST : public LValueAST
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
	Value* accept(ASTVisitor& visitor) override;
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
	Value* accept(ASTVisitor& visitor) override;
};

class UnaryExprAST : public ExprAST
{
public:
	std::string op;
	std::unique_ptr<ExprAST> operand;

	UnaryExprAST(const std::string& op, std::unique_ptr<ExprAST> operand)
		: op(op), operand(std::move(operand))
	{
	}

	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "UnaryExpr: " << op << "\n";
		if (operand) operand->print(indent + 2);
	}

	Value* accept(ASTVisitor& visitor) override;
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
	Value* accept(ASTVisitor& visitor) override;
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
	Value* accept(ASTVisitor& visitor) override;
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
	Value* accept(ASTVisitor& visitor) override;
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
	Value* accept(ASTVisitor& visitor) override;
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
	Value* accept(ASTVisitor& visitor) override;
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
	Value* accept(ASTVisitor& visitor) override;
};

// ForStmtAST 表示一个 for 循环语句
class ForStmtAST : public StmtAST
{
public:
	std::unique_ptr<StmtAST> init;       // 初始语句，例如 int i = 0; 或 i = 0;
	std::unique_ptr<ExprAST> condition;  // 条件表达式，例如 i < 10
	std::unique_ptr<StmtAST> increment;  // 步进语句，例如 i = i + 1
	std::unique_ptr<StmtAST> body;       // 循环体

	ForStmtAST(std::unique_ptr<StmtAST> init, std::unique_ptr<ExprAST> condition,
		std::unique_ptr<StmtAST> increment, std::unique_ptr<StmtAST> body)
		: init(std::move(init)), condition(std::move(condition)),
		increment(std::move(increment)), body(std::move(body))
	{
	}

	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "ForStmt:\n";
		if (init)
		{
			std::cout << indentStr << "  Init:\n";
			init->print(indent + 4);
		}
		if (condition)
		{
			std::cout << indentStr << "  Condition:\n";
			condition->print(indent + 4);
		}
		if (increment)
		{
			std::cout << indentStr << "  Increment:\n";
			increment->print(indent + 4);
		}
		if (body)
		{
			std::cout << indentStr << "  Body:\n";
			body->print(indent + 4);
		}
	}

	Value* accept(ASTVisitor& visitor) override;
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
	Value* accept(ASTVisitor& visitor) override;
};

// VarDeclStmtAST 表示一个变量声明语句，例如 int x = 10;
class VarDeclStmtAST : public StmtAST
{
public:
	std::string varType;
	std::string varName;	// 这个并非等同于VariableExprAST,因为一个是变量声明,另一个是变量使用,虽然它们都包含一个名字,但它们在语义上是不同的,所以我们需要两个不同的类来表示它们
	std::unique_ptr<ExprAST> initValue; // 可选的初始化表达式
	bool isConst;
	VarDeclStmtAST(const std::string& varType, const std::string& varName, std::unique_ptr<ExprAST> initValue = nullptr, bool isConst = false)
		: varType(varType), varName(varName), initValue(std::move(initValue)), isConst(isConst)
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "VarDeclStmt: " << (isConst ? "const " : "") << varType << " " << varName << "\n";
		if (initValue)
		{
			std::cout << indentStr << "Initializer:\n";
			initValue->print(indent + 2);
		}
	}
	Value* accept(ASTVisitor& visitor) override;
};

class DeclGroupAST : public StmtAST
{
public:
	std::vector<std::unique_ptr<StmtAST>> declarations;

	DeclGroupAST(std::vector<std::unique_ptr<StmtAST>> declarations)
		: declarations(std::move(declarations))
	{
	}

	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "DeclGroup:\n";
		for (const auto& decl : declarations)
		{
			if (decl) decl->print(indent + 2);
		}
	}

	Value* accept(ASTVisitor& visitor) override;
};
//======================================================================
// 其他语句子类,例如函数声明、函数定义、类声明等,这些语句通常会包含一个名字、一个类型、一个参数列表和一个函数体等信息,它们在语义上是比较复杂的,所以我们需要单独的类来表示它们

// 函数定义
class FunctionDeclAST : public StmtAST
{
public:
	std::string returnType;
	std::string functionName;
	std::vector<FunctionParamAST> parameters;
	std::unique_ptr<BlockStmtAST> body; // 函数体
	FunctionDeclAST(const std::string& returnType, const std::string& functionName,
		std::vector<FunctionParamAST> parameters,
		std::unique_ptr<BlockStmtAST> body)
		: returnType(returnType), functionName(functionName), parameters(std::move(parameters)), body(std::move(body))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "FunctionDecl: " << returnType << " " << functionName << "(";
		for (size_t i = 0; i < parameters.size(); ++i)
		{
			std::cout << parameters[i].type << " " << parameters[i].name;
			if (parameters[i].isArray) std::cout << "[]";
			if (i < parameters.size() - 1)
				std::cout << ", ";
		}
		std::cout << ")\n";
		if (body) body->print(indent + 2);
	}
	Value* accept(ASTVisitor& visitor) override;
};

// 函数声明
class FunctionDeclStmtAST : public StmtAST
{
public:
	std::string returnType;
	std::string functionName;
	std::vector<FunctionParamAST> parameters;
	FunctionDeclStmtAST(const std::string& returnType, const std::string& functionName,
		std::vector<FunctionParamAST> parameters)
		: returnType(returnType), functionName(functionName), parameters(std::move(parameters))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "FunctionDeclStmt: " << returnType << " " << functionName << "(";
		for (size_t i = 0; i < parameters.size(); ++i)
		{
			std::cout << parameters[i].type << " " << parameters[i].name;
			if (parameters[i].isArray) std::cout << "[]";
			if (i < parameters.size() - 1)
				std::cout << ", ";
		}
		std::cout << ")\n";
	}
	Value* accept(ASTVisitor& visitor) override;
};

// CallExprAST表示函数调用
class CallExprAST : public ExprAST
{
public:
	std::string name;
	std::vector<std::unique_ptr<ExprAST>> args;
	int sourceLine = 0;
	CallExprAST(const std::string& name, std::vector<std::unique_ptr<ExprAST>> args, int sourceLine = 0)
		: name(name), args(std::move(args)), sourceLine(sourceLine)
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
	Value* accept(ASTVisitor& visitor) override;
};

// 数组声明，例如 int arr[5][10] = {{1, 2}, {3, 4}};
class ArrayDeclAST : public StmtAST
{
public:
	std::string elementType; // 元素类型，例如 "int"
	std::string arrayName;   // 数组名，例如 "arr"

	// 扁平化存储多维度的 ExprAST (例如存着代表 5 和 10 的节点)
	std::vector<std::unique_ptr<ExprAST>> dimensions;

	// 初始化列表。如果没有初始化则为 nullptr
	std::unique_ptr<ExprAST> initVal;
	bool isConst;

	// 构造函数：由于涉及 unique_ptr，必须使用 std::move 转移所有权
	ArrayDeclAST(const std::string& elementType, const std::string& arrayName,
		std::vector<std::unique_ptr<ExprAST>> dims,
		std::unique_ptr<ExprAST> init = nullptr,
		bool isConst = false)
		: elementType(elementType), arrayName(arrayName),
		dimensions(std::move(dims)), initVal(std::move(init)), isConst(isConst)
	{
	}

	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "ArrayDecl: " << (isConst ? "const " : "") << elementType << " " << arrayName << "\n";

		// 打印所有的维度
		for (size_t i = 0; i < dimensions.size(); ++i)
		{
			std::cout << indentStr << "  [Dim " << i << "]:\n";
			if (dimensions[i])
			{
				dimensions[i]->print(indent + 4);
			}
		}

		// 如果有初始化列表，打印它
		if (initVal)
		{
			std::cout << indentStr << "  InitValue:\n";
			initVal->print(indent + 4);
		}
	}

	Value* accept(ASTVisitor& visitor) override;
};


// 数组访问表达式，例如: arr[i][j+1]
class ArrayAccessExprAST : public LValueAST
{
public:
	std::string arrayName; // 数组的基准名字，例如 "arr"

	// 扁平化存储：按顺序存放所有的索引表达式
	// 对于 arr[i][j+1]，这里面就存着 i 和 j+1 对应的 ExprAST
	std::vector<std::unique_ptr<ExprAST>> indices;

	// 构造函数：由于 std::unique_ptr 不能拷贝，必须使用 std::move 转移所有权
	ArrayAccessExprAST(const std::string& name, std::vector<std::unique_ptr<ExprAST>> idxs)
		: arrayName(name), indices(std::move(idxs))
	{
	}

	// 打印调试信息，方便你在终端查看语法树的层级
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "ArrayAccessExpr: " << arrayName << "\n";

		// 遍历打印出所有的维度索引
		for (size_t i = 0; i < indices.size(); ++i)
		{
			std::cout << indentStr << "  [Dim " << i << "]:\n";
			if (indices[i])
			{
				indices[i]->print(indent + 4);
			}
		}
	}

	// 访问者模式接口，记得在 ASTVisitor.h 中加上对应的纯虚函数
	Value* accept(ASTVisitor& visitor) override;
};
// 专门用来处理大括号 {} 的初始化列表
class InitListExprAST : public ExprAST
{
public:
	// 列表里的元素。
	// 注意：它们本身也可以是 InitListExprAST（处理多维嵌套初始化）
	std::vector<std::unique_ptr<ExprAST>> elements;

	InitListExprAST(std::vector<std::unique_ptr<ExprAST>> elems)
		: elements(std::move(elems))
	{
	}
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "InitListExpr: {\n";
		for (size_t i = 0; i < elements.size(); ++i)
		{
			std::cout << indentStr << "  (Element " << i << "):\n";
			if (elements[i])
			{
				elements[i]->print(indent + 4);
			}
		}
		std::cout << indentStr << "}\n";
	}
	Value* accept(ASTVisitor& visitor) override;
};
// 结构体声明, 例如 struct Point { int x; int y; };
class StructDeclAST : public StmtAST
{
public:
	std::string structName;
	std::vector<std::unique_ptr<VarDeclStmtAST>> fields; // 结构体的字段列表，使用 VarDeclStmtAST 来表示每个字段的类型和名字
	StructDeclAST(const std::string& structName, std::vector<std::unique_ptr<VarDeclStmtAST>> fields)
		: structName(structName), fields(std::move(fields))
	{
	}
	/*void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "StructDecl: " << structName << "\n";
		
	}*/
	Value* accept(ASTVisitor& visitor) override;
};


// MemberAccessExprAST 表示成员访问, 例如 obj.field
class MemberAccessExprAST : public ExprAST
{
public:
	std::unique_ptr<ExprAST> object;
	std::string memberName; 

	MemberAccessExprAST(std::unique_ptr<ExprAST> object, const std::string& memberName)
		: object(std::move(object)), memberName(memberName)
	{
	}

	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "MemberAccessExpr: ." << memberName << "\n";
		if (object) object->print(indent + 2);
	}
	Value* accept(ASTVisitor& visitor) override;
};

class EmptyStmtAST : public StmtAST
{
public:
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "EmptyStmt\n";
	}
	Value* accept(ASTVisitor& visitor) override;
};

class AssignStmtAST : public StmtAST
{
public:
	std::unique_ptr<LValueAST> target;
	std::unique_ptr<ExprAST> value;

	AssignStmtAST(std::unique_ptr<LValueAST> target, std::unique_ptr<ExprAST> value)
		: target(std::move(target)), value(std::move(value))
	{
	}

	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "AssignStmt:\n";
		if (target)
		{
			std::cout << indentStr << "  Target:\n";
			target->print(indent + 4);
		}
		if (value)
		{
			std::cout << indentStr << "  Value:\n";
			value->print(indent + 4);
		}
	}

	Value* accept(ASTVisitor& visitor) override;
};

class BreakStmtAST : public StmtAST
{
public:
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "BreakStmt\n";
	}
	Value* accept(ASTVisitor& visitor) override;
};

class ContinueStmtAST : public StmtAST
{
public:
	void print(int indent = 0) const override
	{
		std::string indentStr(indent, ' ');
		std::cout << indentStr << "ContinueStmt\n";
	}
	Value* accept(ASTVisitor& visitor) override;
};
