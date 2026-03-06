#include "SemanticAnalyzer.h"
#include <iostream>
#include <algorithm>

// 有点懵逼,再品品的
bool SemanticAnalyzer::analyze(ProgramAST* roots)
{
	enterScope(); // 进入全局作用域
	// 假设 root 是一个语句 (StmtAST) 或者函数定义
	//for (StmtAST* root : (roots->declarations))
	for (int i = 0; i < roots->declarations.size(); i++)
	{
		std::cout << "Analyzing declaration " << i + 1 << "/" << roots->declarations.size() << "...\n";
		StmtAST* root = roots->declarations[i].get();
		if (auto* stmt = dynamic_cast<StmtAST*>(root))
		{
			analyzeStmt(stmt);
		}
		else if (auto* func = dynamic_cast<FunctionDeclAST*>(root))
		{
			analyzeFunctionDecl(func);
		}
	}

	auto it = symbolTableStack.front().find("main()"); // 查找全局作用域中的 main 函数
	if (it == symbolTableStack.front().end())
	{
		std::cerr << "Semantic Error: Where is your main function? Where should I enter? Your butt? \n";
		hasError = true;
	}

	exitScope(); // 退出全局作用域
	return hasError;
}

void SemanticAnalyzer::enterScope()
{
	symbolTableStack.push_back({});
}

void SemanticAnalyzer::exitScope()
{
	if (!symbolTableStack.empty())
	{
		symbolTableStack.pop_back();
	}
	else
	{
		std::cerr << "Semantic Error: No scope to exit\n";
		hasError = true;
	}
}

// 语句分发
void SemanticAnalyzer::analyzeStmt(StmtAST* stmt)
{
	if (!stmt) return;

	if (auto* it = dynamic_cast<VarDeclStmtAST*>(stmt))
	{
		analyzeVarDecl(it);
	}
	else if (auto* it = dynamic_cast<VariableExprAST*>(stmt))
	{
		analyzeVariableExpr(it);
	}
	else if (auto* it = dynamic_cast<ReturnStmtAST*>(stmt))
	{
		analyzeExpr(it->returnValue.get());
	}
	else if (auto* it = dynamic_cast<BlockStmtAST*>(stmt))
	{
		enterScope(); // 进入 {} 块级作用域
		for (const auto& s : it->statements)
		{
			analyzeStmt(s.get());
		}
		exitScope();  // 离开块级作用域
	}
	else if (auto* it = dynamic_cast<IfStmtAST*>(stmt))
	{
		analyzeIfStmt(it);
	}
	else if (auto* it = dynamic_cast<WhileStmtAST*>(stmt))
	{
		analyzeWhileStmt(it);
	}
	else if (auto* it = dynamic_cast<DoWhileStmtAST*>(stmt))
	{
		analyzeDoWhileStmt(it);
	}
	else if (auto* it = dynamic_cast<ExprStmtAST*>(stmt))
	{
		analyzeExpr(it->expr.get());
	}
	else if (auto* it = dynamic_cast<FunctionDeclAST*>(stmt))
	{
		analyzeFunctionDecl(it);
	}
	else
	{
		std::cerr << "Semantic Error: Unknown statement type\n";
		hasError = true;
	}
}

std::string SemanticAnalyzer::analyzeExpr(ExprAST* expr)
{
	if (!expr) return "void";

	// 处理数字字面量
	// 目前只做了int,其他做不动了
	// 要做应该也行,但是现在没做'.',f也没有做,就都不做了
	if (auto* numExpr = dynamic_cast<NumberExprAST*>(expr))
	{
		return "int";
	}

	if (auto* strExpr = dynamic_cast<StringExprAST*>(expr))
	{
		return "string"; // 返回自定义的类型名
	}

	// 处理变量使用
	if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr))
	{
		return analyzeVariableExpr(varExpr);
	}

	// 处理二元运算
	if (auto* binExpr = dynamic_cast<BinaryExprAST*>(expr))
	{
		std::string leftType = analyzeExpr(binExpr->LHS.get());
		std::string rightType = analyzeExpr(binExpr->RHS.get());

		// 简单校验：左右类型不匹配且不为unknown时报错
		if (leftType != "unknown" && rightType != "unknown" && leftType != rightType)
		{
			std::cerr << "Semantic Error: Type mismatch in binary operation '"
				<< binExpr->op << "'\n";
			hasError = true;
		}
		return leftType;
	}

	// 处理函数调用
	// 好像正常来说判断参数相不相同是要先看数量再类型,然后还有参数列表,但是我只存了string,那我直接按照函数重载那一套来判断参数可不可以了
	if (auto* callExpr = dynamic_cast<CallExprAST*>(expr))
	{
		if (callExpr->name == "printf")
		{
			for (const auto& arg : callExpr->args)
			{
				analyzeExpr(arg.get()); // 递归检查内部参数
			}
			return "int"; // printf 默认返回 int
		}
		std::vector<std::string> argTypes;
		for (const auto& arg : callExpr->args) 
		{
			std::string argType = analyzeExpr(arg.get()); // 递归计算参数类型
			if (argType == "unknown" || argType == "void")
			{
				// 如果某个参数本身就是错的，直接终止当前函数的解析
				return "unknown";
			}
			argTypes.push_back(argType);
		}

		// 根据推导出的实参类型，拼装带有参数签名的函数名
		std::string mangledName = callExpr->name + "(";
		for (size_t i = 0; i < argTypes.size(); ++i)
		{
			mangledName += argTypes[i];
			if (i < argTypes.size() - 1)
			{
				mangledName += ",";
			}
		}
		mangledName += ")";
		if (mangledName == "main()")
		{
			std::cerr << "Semantic Error: HAHAHASB! Call the main function? It's kind of interesting.\n";
			hasError = true;
			return "unknown";
		}
		for (auto it = symbolTableStack.rbegin(); it != symbolTableStack.rend(); ++it)
		{
			auto funcIt = it->find(mangledName);
			if (funcIt != it->end())
			{
				// 找到了完全匹配的函数（名字相同，参数数量相同，参数类型也相同）
				return funcIt->second;
			}
		}

		std::cerr << "Semantic Error: No matching function for call to '"
			<< callExpr->name << "' with the provided arguments.\n";
		hasError = true;
		return "unknown";
	}

	return "unknown";
}

void SemanticAnalyzer::analyzeVarDecl(VarDeclStmtAST* varDecl)
{
	auto it = symbolTableStack.back().find(varDecl->varName);

	if (it != symbolTableStack.back().end())
	{
		std::cerr << "Semantic Error: Variable '" << varDecl->varName << "' already declared in this scope\n";
		hasError = true;
	}
	else
	{
		// 如果有初始化表达式 (比如 int x = 10;)，检查等号右边的类型
		if (varDecl->initValue)
		{
			std::string initType = analyzeExpr(varDecl->initValue.get());
			if (initType != "unknown" && initType != varDecl->varType)
			{
				std::cerr << "Semantic Error: Cannot initialize variable of type '"
					<< varDecl->varType << "' with type '" << initType << "'\n";
				hasError = true;
			}
		}
		// 登记变量
		symbolTableStack.back()[varDecl->varName] = varDecl->varType;
	}
}

std::string SemanticAnalyzer::analyzeVariableExpr(VariableExprAST* varExpr)
{
	for (auto it = symbolTableStack.rbegin(); it != symbolTableStack.rend(); ++it)
	{
		auto varIt = it->find(varExpr->name);
		if (varIt != it->end())
		{
			return varIt->second;
		}
	}

	std::cerr << "Semantic Error: Variable '" << varExpr->name << "' not declared\n";
	hasError = true;
	return "unknown";
}

// 函数存储名字形似func(int,int)
void SemanticAnalyzer::analyzeFunctionDecl(FunctionDeclAST* funcDecl)
{
	std::string mangledName = funcDecl->functionName + "(";
	for (size_t i = 0; i < funcDecl->parameters.size(); ++i)
	{
		mangledName += funcDecl->parameters[i].first;
		if (i < funcDecl->parameters.size() - 1)
		{
			mangledName += ",";
		}
	}
	mangledName += ")";
	if (mangledName == "main()")
	{
		hasMainFunction = true;
	}
	auto it = symbolTableStack.back().find(mangledName);
	if (it != symbolTableStack.back().end())
	{
		std::cerr << "Semantic Error: Function '" << mangledName << "' already declared\n";
		hasError = true;
	}
	else
	{
		symbolTableStack.back()[mangledName] = funcDecl->returnType;
	}

	enterScope();

	for (const auto& param : funcDecl->parameters)
	{
		if (symbolTableStack.back().count(param.second))
		{
			std::cerr << "Semantic Error: Parameter '" << param.second << "' redefinition\n";
			hasError = true;
		}
		symbolTableStack.back()[param.second] = param.first;
	}

	if (funcDecl->body)
	{
		for (const auto& stmt : funcDecl->body->statements) 
		{
			analyzeStmt(stmt.get());
		}
	}

	exitScope();
}

void SemanticAnalyzer::analyzeIfStmt(IfStmtAST* ifStmt)
{
	std::string condType = analyzeExpr(ifStmt->condition.get());

	if (condType != "unknown" && condType == "void")
	{
		std::cerr << "Semantic Error: 'if' condition cannot be of type 'void'\n";
		hasError = true;
	}
	if (ifStmt->thenBranch)
	{
		analyzeStmt(ifStmt->thenBranch.get());
	}

	if (ifStmt->elseBranch)
	{
		analyzeStmt(ifStmt->elseBranch.get());
	}
}

void SemanticAnalyzer::analyzeWhileStmt(WhileStmtAST* whileStmt)
{
	std::string condType = analyzeExpr(whileStmt->condition.get());

	if (condType != "unknown" && condType == "void")
	{
		std::cerr << "Semantic Error: 'while' condition cannot be of type 'void'\n";
		hasError = true;
	}

	if (whileStmt->body)
	{
		analyzeStmt(whileStmt->body.get());
	}
}

void SemanticAnalyzer::analyzeDoWhileStmt(DoWhileStmtAST* doWhileStmt)
{
	if (doWhileStmt->body)
	{
		analyzeStmt(doWhileStmt->body.get());
	}
	std::string condType = analyzeExpr(doWhileStmt->condition.get());
	if (condType != "unknown" && condType == "void")
	{
		std::cerr << "Semantic Error: 'do-while' condition cannot be of type 'void'\n";
		hasError = true;
	}
}