#include "frontend/sema/SemanticAnalyzer.h"

#include <cstddef>
#include <iostream>

static bool isFloatLiteral(const std::string& value)
{
	return value.find('.') != std::string::npos
		|| value.find('e') != std::string::npos
		|| value.find('E') != std::string::npos
		|| value.find('p') != std::string::npos
		|| value.find('P') != std::string::npos
		|| value.find('f') != std::string::npos
		|| value.find('F') != std::string::npos;
}

bool SemanticAnalyzer::analyze(ProgramAST* root)
{
	hasError = false;
	hasMainFunction = false;
	loopDepth = 0;
	currentReturnType.clear();
	symbolTableStack.clear();
	functions.clear();
	knownConstInts.clear();

	enterScope();
	registerBuiltinFunctions();
	for (const auto& decl : root->declarations)
	{
		collectGlobalConstInt(decl.get());
	}
	for (const auto& decl : root->declarations)
	{
		collectFunctionSignature(decl.get());
	}

	for (const auto& decl : root->declarations)
	{
		analyzeStmt(decl.get());
	}

	if (!hasMainFunction)
	{
		std::cerr << "Semantic Error: missing main function\n";
		hasError = true;
	}

	exitScope();
	return !hasError;
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
}

bool SemanticAnalyzer::isArrayType(const std::string& type) const
{
	return type.size() >= 2 && type.substr(type.size() - 2) == "[]";
}

std::string SemanticAnalyzer::baseTypeOf(const std::string& type) const
{
	return isArrayType(type) ? type.substr(0, type.size() - 2) : type;
}

bool SemanticAnalyzer::isNumericType(const std::string& type) const
{
	return type == "int" || type == "float";
}

std::string SemanticAnalyzer::usualArithmeticType(const std::string& left, const std::string& right) const
{
	if (!isNumericType(left) || !isNumericType(right))
	{
		return "unknown";
	}
	if (left == "float" || right == "float")
	{
		return "float";
	}
	return "int";
}

bool SemanticAnalyzer::isConditionOperator(const std::string& op) const
{
	return op == "==" || op == "!=" || op == "<" || op == "<="
		|| op == ">" || op == ">=" || op == "&&" || op == "||";
}

bool SemanticAnalyzer::isAssignableType(const std::string& left, const std::string& right) const
{
	if (left == "unknown" || right == "unknown")
	{
		return true;
	}
	if (isArrayType(left) || isArrayType(right))
	{
		return left == right;
	}
	if (isNumericType(left) && isNumericType(right))
	{
		return true;
	}
	return left == right;
}

SemanticAnalyzer::SymbolInfo* SemanticAnalyzer::lookupSymbol(const std::string& name)
{
	for (auto it = symbolTableStack.rbegin(); it != symbolTableStack.rend(); ++it)
	{
		auto found = it->find(name);
		if (found != it->end())
		{
			return &found->second;
		}
	}
	return nullptr;
}

const SemanticAnalyzer::SymbolInfo* SemanticAnalyzer::lookupSymbol(const std::string& name) const
{
	for (auto it = symbolTableStack.rbegin(); it != symbolTableStack.rend(); ++it)
	{
		auto found = it->find(name);
		if (found != it->end())
		{
			return &found->second;
		}
	}
	return nullptr;
}

bool SemanticAnalyzer::isTopLevelContext() const
{
	return symbolTableStack.size() == 1 && currentReturnType.empty();
}

bool SemanticAnalyzer::guaranteedReturns(StmtAST* stmt) const
{
	if (!stmt)
	{
		return false;
	}
	if (dynamic_cast<ReturnStmtAST*>(stmt))
	{
		return true;
	}
	if (auto* block = dynamic_cast<BlockStmtAST*>(stmt))
	{
		for (const auto& child : block->statements)
		{
			if (guaranteedReturns(child.get()))
			{
				return true;
			}
		}
		return false;
	}
	if (auto* group = dynamic_cast<DeclGroupAST*>(stmt))
	{
		for (const auto& child : group->declarations)
		{
			if (guaranteedReturns(child.get()))
			{
				return true;
			}
		}
		return false;
	}
	if (auto* ifStmt = dynamic_cast<IfStmtAST*>(stmt))
	{
		return ifStmt->thenBranch && ifStmt->elseBranch
			&& guaranteedReturns(ifStmt->thenBranch.get())
			&& guaranteedReturns(ifStmt->elseBranch.get());
	}
	return false;
}

int SemanticAnalyzer::evalConstInt(ExprAST* expr) const
{
	if (auto* number = dynamic_cast<NumberExprAST*>(expr))
	{
		return std::stoi(number->value, nullptr, 0);
	}
	if (auto* variable = dynamic_cast<VariableExprAST*>(expr))
	{
		const SymbolInfo* symbol = lookupSymbol(variable->name);
		if (symbol && symbol->hasConstIntValue)
		{
			return symbol->constIntValue;
		}
		auto known = knownConstInts.find(variable->name);
		return known != knownConstInts.end() ? known->second : 0;
	}
	if (auto* unary = dynamic_cast<UnaryExprAST*>(expr))
	{
		int value = evalConstInt(unary->operand.get());
		if (unary->op == "-") return -value;
		if (unary->op == "!") return !value;
		return value;
	}
	if (auto* binary = dynamic_cast<BinaryExprAST*>(expr))
	{
		int lhs = evalConstInt(binary->LHS.get());
		int rhs = evalConstInt(binary->RHS.get());
		if (binary->op == "+") return lhs + rhs;
		if (binary->op == "-") return lhs - rhs;
		if (binary->op == "*") return lhs * rhs;
		if (binary->op == "/" && rhs != 0) return lhs / rhs;
		if (binary->op == "%" && rhs != 0) return lhs % rhs;
		if (binary->op == "<") return lhs < rhs;
		if (binary->op == "<=") return lhs <= rhs;
		if (binary->op == ">") return lhs > rhs;
		if (binary->op == ">=") return lhs >= rhs;
		if (binary->op == "==") return lhs == rhs;
		if (binary->op == "!=") return lhs != rhs;
		if (binary->op == "&&") return lhs && rhs;
		if (binary->op == "||") return lhs || rhs;
	}
	return 0;
}

bool SemanticAnalyzer::isConstIntExpr(ExprAST* expr) const
{
	if (!expr)
	{
		return false;
	}
	if (auto* number = dynamic_cast<NumberExprAST*>(expr))
	{
		return !isFloatLiteral(number->value);
	}
	if (auto* variable = dynamic_cast<VariableExprAST*>(expr))
	{
		const SymbolInfo* symbol = lookupSymbol(variable->name);
		if (symbol && symbol->hasConstIntValue)
		{
			return true;
		}
		return knownConstInts.find(variable->name) != knownConstInts.end();
	}
	if (auto* unary = dynamic_cast<UnaryExprAST*>(expr))
	{
		return (unary->op == "+" || unary->op == "-" || unary->op == "!")
			&& isConstIntExpr(unary->operand.get());
	}
	if (auto* binary = dynamic_cast<BinaryExprAST*>(expr))
	{
		if (binary->op != "+" && binary->op != "-" && binary->op != "*" && binary->op != "/"
			&& binary->op != "%" && binary->op != "<" && binary->op != "<="
			&& binary->op != ">" && binary->op != ">=" && binary->op != "=="
			&& binary->op != "!=" && binary->op != "&&" && binary->op != "||")
		{
			return false;
		}
		return isConstIntExpr(binary->LHS.get()) && isConstIntExpr(binary->RHS.get());
	}
	return false;
}

bool SemanticAnalyzer::isConstLValue(LValueAST* lvalue) const
{
	std::string name;
	if (auto* var = dynamic_cast<VariableExprAST*>(lvalue))
	{
		name = var->name;
	}
	else if (auto* access = dynamic_cast<ArrayAccessExprAST*>(lvalue))
	{
		name = access->arrayName;
	}
	const auto* symbol = lookupSymbol(name);
	return symbol && symbol->isConst;
}

std::vector<int> SemanticAnalyzer::decayedArrayDimensions(ExprAST* expr) const
{
	std::vector<int> residual;
	if (auto* var = dynamic_cast<VariableExprAST*>(expr))
	{
		const SymbolInfo* symbol = lookupSymbol(var->name);
		if (!symbol || !symbol->isArray)
		{
			return {};
		}
		if (symbol->isArrayParam)
		{
			return symbol->dimensions;
		}
		if (symbol->dimensions.size() > 1)
		{
			return std::vector<int>(symbol->dimensions.begin() + 1, symbol->dimensions.end());
		}
		return {};
	}
	if (auto* access = dynamic_cast<ArrayAccessExprAST*>(expr))
	{
		const SymbolInfo* symbol = lookupSymbol(access->arrayName);
		if (!symbol || !symbol->isArray)
		{
			return {};
		}
		std::vector<int> fullDimensions = symbol->dimensions;
		if (symbol->isArrayParam)
		{
			fullDimensions.insert(fullDimensions.begin(), -1);
		}
		if (access->indices.size() >= fullDimensions.size())
		{
			return {};
		}
		residual.assign(fullDimensions.begin() + static_cast<std::ptrdiff_t>(access->indices.size()),
			fullDimensions.end());
		if (!residual.empty())
		{
			residual.erase(residual.begin());
		}
	}
	return residual;
}

void SemanticAnalyzer::registerBuiltinFunctions()
{
	auto add = [this](const std::string& name, const std::string& ret,
		std::vector<std::string> params, std::vector<bool> arrays = {})
	{
		if (arrays.empty())
		{
			arrays.resize(params.size(), false);
		}
		std::vector<std::vector<int>> dimensions(params.size());
		addFunctionSignature(name, { ret, std::move(params), std::move(arrays), std::move(dimensions), true, false });
	};

	add("getint", "int", {});
	add("getch", "int", {});
	add("getfloat", "float", {});
	add("getarray", "int", { "int" }, { true });
	add("getfarray", "int", { "float" }, { true });
	add("putint", "void", { "int" });
	add("putch", "void", { "int" });
	add("putfloat", "void", { "float" });
	add("putarray", "void", { "int", "int" }, { false, true });
	add("putfarray", "void", { "int", "float" }, { false, true });
	add("_sysy_starttime", "void", { "int" });
	add("_sysy_stoptime", "void", { "int" });
}

void SemanticAnalyzer::collectGlobalConstInt(StmtAST* stmt)
{
	if (auto* group = dynamic_cast<DeclGroupAST*>(stmt))
	{
		for (const auto& decl : group->declarations)
		{
			collectGlobalConstInt(decl.get());
		}
		return;
	}
	auto* varDecl = dynamic_cast<VarDeclStmtAST*>(stmt);
	if (!varDecl || !varDecl->isConst || varDecl->varType != "int" || !varDecl->initValue)
	{
		return;
	}
	if (isConstIntExpr(varDecl->initValue.get()))
	{
		knownConstInts[varDecl->varName] = evalConstInt(varDecl->initValue.get());
	}
}

void SemanticAnalyzer::addFunctionSignature(const std::string& name, const FunctionSignature& signature)
{
	auto& list = functions[name];
	bool sawSameName = !list.empty();
	for (auto& existing : list)
	{
		if (existing.paramTypes == signature.paramTypes
			&& existing.paramIsArray == signature.paramIsArray
			&& existing.paramDimensions == signature.paramDimensions)
		{
			if (existing.returnType != signature.returnType)
			{
				std::cerr << "Semantic Error: conflicting return type for function '" << name << "'\n";
				hasError = true;
			}
			else if (existing.isDefinition && signature.isDefinition && !existing.builtin && !signature.builtin)
			{
				std::cerr << "Semantic Error: duplicate function definition '" << name << "'\n";
				hasError = true;
			}
			existing.isDefinition = existing.isDefinition || signature.isDefinition;
			return;
		}
	}
	if (sawSameName)
	{
		std::cerr << "Semantic Error: conflicting function declaration '" << name << "'\n";
		hasError = true;
		return;
	}
	list.push_back(signature);
}

void SemanticAnalyzer::collectFunctionSignature(StmtAST* stmt)
{
	auto collect = [this, stmt](const std::string& name, const std::string& ret,
		const std::vector<FunctionParamAST>& params)
	{
		FunctionSignature signature;
		signature.returnType = ret;
		for (const auto& param : params)
		{
			signature.paramTypes.push_back(param.type);
			signature.paramIsArray.push_back(param.isArray);
			std::vector<int> dimensions;
			for (const auto& dim : param.dimensions)
			{
				dimensions.push_back(isConstIntExpr(dim.get()) ? evalConstInt(dim.get()) : 0);
			}
			signature.paramDimensions.push_back(std::move(dimensions));
		}
		signature.isDefinition = dynamic_cast<FunctionDeclAST*>(stmt) != nullptr;
		addFunctionSignature(name, signature);
		if (name == "main" && params.empty() && ret == "int")
		{
			hasMainFunction = true;
		}
	};

	if (auto* function = dynamic_cast<FunctionDeclAST*>(stmt))
	{
		collect(function->functionName, function->returnType, function->parameters);
	}
	else if (auto* declaration = dynamic_cast<FunctionDeclStmtAST*>(stmt))
	{
		collect(declaration->functionName, declaration->returnType, declaration->parameters);
	}
}

bool SemanticAnalyzer::isFunctionCompatible(const FunctionSignature& signature,
	const std::vector<std::string>& argTypes,
	const std::vector<std::vector<int>>& argArrayDimensions) const
{
	if (signature.paramTypes.size() != argTypes.size())
	{
		return false;
	}
	for (size_t i = 0; i < argTypes.size(); ++i)
	{
		bool argArray = isArrayType(argTypes[i]);
		if (signature.paramIsArray[i] != argArray)
		{
			return false;
		}
		if (signature.paramTypes[i] != baseTypeOf(argTypes[i]))
		{
			if (!(isNumericType(signature.paramTypes[i]) && isNumericType(baseTypeOf(argTypes[i]))))
			{
				return false;
			}
		}
		if (signature.paramIsArray[i] && signature.paramDimensions[i] != argArrayDimensions[i])
		{
			return false;
		}
	}
	return true;
}

const SemanticAnalyzer::FunctionSignature* SemanticAnalyzer::findFunction(
	const std::string& name, const std::vector<std::string>& argTypes) const
{
	std::vector<std::vector<int>> emptyDimensions(argTypes.size());
	return findFunction(name, argTypes, emptyDimensions);
}

const SemanticAnalyzer::FunctionSignature* SemanticAnalyzer::findFunction(
	const std::string& name, const std::vector<std::string>& argTypes,
	const std::vector<std::vector<int>>& argArrayDimensions) const
{
	auto found = functions.find(name);
	if (found == functions.end())
	{
		return nullptr;
	}
	for (const auto& signature : found->second)
	{
		if (isFunctionCompatible(signature, argTypes, argArrayDimensions))
		{
			return &signature;
		}
	}
	return nullptr;
}

void SemanticAnalyzer::analyzeStmt(StmtAST* stmt)
{
	if (!stmt)
	{
		return;
	}

	if (isTopLevelContext()
		&& !dynamic_cast<VarDeclStmtAST*>(stmt)
		&& !dynamic_cast<ArrayDeclAST*>(stmt)
		&& !dynamic_cast<DeclGroupAST*>(stmt)
		&& !dynamic_cast<FunctionDeclAST*>(stmt)
		&& !dynamic_cast<FunctionDeclStmtAST*>(stmt))
	{
		std::cerr << "Semantic Error: SysY top-level item must be declaration or function definition\n";
		hasError = true;
		return;
	}

	if (auto* it = dynamic_cast<VarDeclStmtAST*>(stmt))
	{
		analyzeVarDecl(it);
	}
	else if (auto* it = dynamic_cast<ArrayDeclAST*>(stmt))
	{
		analyzeArrayDecl(it);
	}
	else if (auto* it = dynamic_cast<DeclGroupAST*>(stmt))
	{
		for (const auto& decl : it->declarations)
		{
			analyzeStmt(decl.get());
		}
	}
	else if (dynamic_cast<EmptyStmtAST*>(stmt))
	{
		return;
	}
	else if (auto* it = dynamic_cast<AssignStmtAST*>(stmt))
	{
		if (isConstLValue(it->target.get()))
		{
			std::cerr << "Semantic Error: cannot assign to const lvalue\n";
			hasError = true;
		}
		std::string leftType = analyzeExpr(it->target.get());
		std::string rightType = analyzeExpr(it->value.get());
		if (isArrayType(leftType))
		{
			std::cerr << "Semantic Error: cannot assign to array object\n";
			hasError = true;
		}
		if (!isAssignableType(leftType, rightType))
		{
			std::cerr << "Semantic Error: Type mismatch in assignment\n";
			hasError = true;
		}
	}
	else if (dynamic_cast<BreakStmtAST*>(stmt))
	{
		if (loopDepth == 0)
		{
			std::cerr << "Semantic Error: break used outside loop\n";
			hasError = true;
		}
	}
	else if (dynamic_cast<ContinueStmtAST*>(stmt))
	{
		if (loopDepth == 0)
		{
			std::cerr << "Semantic Error: continue used outside loop\n";
			hasError = true;
		}
	}
	else if (auto* it = dynamic_cast<ReturnStmtAST*>(stmt))
	{
		std::string returnType = analyzeExpr(it->returnValue.get());
		if (!currentReturnType.empty())
		{
			if (currentReturnType == "void" && it->returnValue)
			{
				std::cerr << "Semantic Error: void function should not return a value\n";
				hasError = true;
			}
			else if (currentReturnType != "void" && !it->returnValue)
			{
				std::cerr << "Semantic Error: non-void function must return a value\n";
				hasError = true;
			}
			else if (it->returnValue && !isAssignableType(currentReturnType, returnType))
			{
				std::cerr << "Semantic Error: return type mismatch\n";
				hasError = true;
			}
		}
	}
	else if (auto* it = dynamic_cast<BlockStmtAST*>(stmt))
	{
		enterScope();
		for (const auto& s : it->statements)
		{
			analyzeStmt(s.get());
		}
		exitScope();
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
	else if (auto* it = dynamic_cast<ForStmtAST*>(stmt))
	{
		analyzeForStmt(it);
	}
	else if (auto* it = dynamic_cast<ExprStmtAST*>(stmt))
	{
		std::string exprType = analyzeExpr(it->expr.get());
		if (exprType == "string" || exprType == "initlist")
		{
			std::cerr << "Semantic Error: invalid expression statement type\n";
			hasError = true;
		}
	}
	else if (auto* it = dynamic_cast<FunctionDeclAST*>(stmt))
	{
		analyzeFunctionDecl(it);
	}
	else if (auto* it = dynamic_cast<FunctionDeclStmtAST*>(stmt))
	{
		validateFunctionParameters(it->parameters);
		std::cerr << "Semantic Error: function declaration without body is not part of SysY: '"
			<< it->functionName << "'\n";
		hasError = true;
		return;
	}
	else
	{
		std::cerr << "Semantic Error: Unknown statement type\n";
		hasError = true;
	}
}

std::string SemanticAnalyzer::analyzeExpr(ExprAST* expr, bool allowConditionOps)
{
	if (!expr)
	{
		return "void";
	}
	if (auto* numberExpr = dynamic_cast<NumberExprAST*>(expr))
	{
		return isFloatLiteral(numberExpr->value) ? "float" : "int";
	}
	if (dynamic_cast<StringExprAST*>(expr))
	{
		return "string";
	}
	if (auto* listExpr = dynamic_cast<InitListExprAST*>(expr))
	{
		for (const auto& element : listExpr->elements)
		{
			analyzeExpr(element.get(), allowConditionOps);
		}
		return "initlist";
	}
	if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr))
	{
		return analyzeVariableExpr(varExpr);
	}
	if (auto* arrayExpr = dynamic_cast<ArrayAccessExprAST*>(expr))
	{
		return analyzeArrayAccessExpr(arrayExpr);
	}
	if (auto* unaryExpr = dynamic_cast<UnaryExprAST*>(expr))
	{
		if (unaryExpr->op == "!" && !allowConditionOps)
		{
			std::cerr << "Semantic Error: logical '!' is only allowed in condition expressions\n";
			hasError = true;
			return "unknown";
		}
		std::string operandType = analyzeExpr(unaryExpr->operand.get(), allowConditionOps);
		if (operandType == "void" || isArrayType(operandType))
		{
			std::cerr << "Semantic Error: invalid unary operand\n";
			hasError = true;
			return "unknown";
		}
		return unaryExpr->op == "!" ? "int" : operandType;
	}
	if (auto* binExpr = dynamic_cast<BinaryExprAST*>(expr))
	{
		if (binExpr->op == "=")
		{
			std::cerr << "Semantic Error: assignment is a statement in SysY, not an expression\n";
			hasError = true;
			return "unknown";
		}
		if (binExpr->op == "&" || binExpr->op == "|")
		{
			std::cerr << "Semantic Error: bitwise operator '" << binExpr->op << "' is not part of SysY\n";
			hasError = true;
			return "unknown";
		}
		if (isConditionOperator(binExpr->op) && !allowConditionOps)
		{
			std::cerr << "Semantic Error: condition operator '" << binExpr->op
				<< "' is not allowed in ordinary SysY Exp\n";
			hasError = true;
			return "unknown";
		}
		std::string leftType = analyzeExpr(binExpr->LHS.get(), allowConditionOps);
		std::string rightType = analyzeExpr(binExpr->RHS.get(), allowConditionOps);
		std::string arithmeticType = usualArithmeticType(leftType, rightType);
		if (isArrayType(leftType) || isArrayType(rightType) || arithmeticType == "unknown"
			|| (binExpr->op == "%" && (leftType != "int" || rightType != "int")))
		{
			std::cerr << "Semantic Error: Type mismatch in binary operation '" << binExpr->op << "'\n";
			hasError = true;
			return "unknown";
		}
		if (binExpr->op == "==" || binExpr->op == "!=" || binExpr->op == "<" || binExpr->op == "<="
			|| binExpr->op == ">" || binExpr->op == ">=" || binExpr->op == "&&" || binExpr->op == "||")
		{
			return "int";
		}
		return arithmeticType;
	}
	if (auto* callExpr = dynamic_cast<CallExprAST*>(expr))
	{
		if (callExpr->name == "starttime" || callExpr->name == "stoptime")
		{
			if (!callExpr->args.empty())
			{
				std::cerr << "Semantic Error: " << callExpr->name << "() takes no SysY arguments\n";
				hasError = true;
				return "unknown";
			}
			return "void";
		}
		if (callExpr->name == "putf")
		{
			if (callExpr->args.empty() || !dynamic_cast<StringExprAST*>(callExpr->args[0].get()))
			{
				std::cerr << "Semantic Error: putf requires a string literal as first argument\n";
				hasError = true;
				return "unknown";
			}
			for (size_t i = 1; i < callExpr->args.size(); ++i)
			{
				std::string argType = analyzeExpr(callExpr->args[i].get());
				if (!isNumericType(argType))
				{
					std::cerr << "Semantic Error: putf variadic arguments must be int or float expressions\n";
					hasError = true;
				}
			}
			return "void";
		}
		std::vector<std::string> argTypes;
		std::vector<std::vector<int>> argDimensions;
		for (const auto& arg : callExpr->args)
		{
			std::string argType = analyzeExpr(arg.get());
			if (argType == "unknown" || argType == "void")
			{
				return "unknown";
			}
			argDimensions.push_back(isArrayType(argType) ? decayedArrayDimensions(arg.get()) : std::vector<int>{});
			argTypes.push_back(argType);
		}
		if (callExpr->name == "main")
		{
			std::cerr << "Semantic Error: calling main is not allowed\n";
			hasError = true;
			return "unknown";
		}
		const FunctionSignature* signature = findFunction(callExpr->name, argTypes, argDimensions);
		if (!signature)
		{
			std::cerr << "Semantic Error: No matching function for call to '" << callExpr->name << "'\n";
			hasError = true;
			return "unknown";
		}
		return signature->returnType;
	}
	return "unknown";
}

void SemanticAnalyzer::analyzeVarDecl(VarDeclStmtAST* varDecl)
{
	if (varDecl->varType == "void")
	{
		std::cerr << "Semantic Error: variable '" << varDecl->varName << "' cannot have void type\n";
		hasError = true;
		return;
	}
	if (symbolTableStack.back().count(varDecl->varName))
	{
		std::cerr << "Semantic Error: Variable '" << varDecl->varName << "' already declared in this scope\n";
		hasError = true;
		return;
	}
	if (varDecl->initValue)
	{
		std::string initType = analyzeExpr(varDecl->initValue.get());
		if (!isAssignableType(varDecl->varType, initType))
		{
			std::cerr << "Semantic Error: Cannot initialize variable of type '"
				<< varDecl->varType << "' with type '" << initType << "'\n";
			hasError = true;
		}
	}
	else if (varDecl->isConst)
	{
		std::cerr << "Semantic Error: const variable '" << varDecl->varName << "' must be initialized\n";
		hasError = true;
	}
	SymbolInfo symbol{ varDecl->varType, varDecl->isConst, false, false, {} };
	if (varDecl->isConst && varDecl->varType == "int" && varDecl->initValue && isConstIntExpr(varDecl->initValue.get()))
	{
		symbol.hasConstIntValue = true;
		symbol.constIntValue = evalConstInt(varDecl->initValue.get());
	}
	symbolTableStack.back()[varDecl->varName] = symbol;
}

void SemanticAnalyzer::analyzeArrayDecl(ArrayDeclAST* arrayDecl)
{
	if (arrayDecl->elementType == "void")
	{
		std::cerr << "Semantic Error: array '" << arrayDecl->arrayName << "' cannot have void element type\n";
		hasError = true;
		return;
	}
	if (symbolTableStack.back().count(arrayDecl->arrayName))
	{
		std::cerr << "Semantic Error: Array '" << arrayDecl->arrayName << "' already declared in this scope\n";
		hasError = true;
		return;
	}
	std::vector<int> dimensions;
	for (const auto& dim : arrayDecl->dimensions)
	{
		std::string dimType = analyzeExpr(dim.get());
		if (dimType != "unknown" && dimType != "int")
		{
			std::cerr << "Semantic Error: Array dimension must be int\n";
			hasError = true;
		}
		if (!isConstIntExpr(dim.get()))
		{
			std::cerr << "Semantic Error: Array dimension must be constant int expression\n";
			hasError = true;
		}
		int value = evalConstInt(dim.get());
		if (value <= 0)
		{
			std::cerr << "Semantic Error: Array dimension must be positive\n";
			hasError = true;
		}
		dimensions.push_back(value);
	}
	if (arrayDecl->initVal)
	{
		validateArrayInitializer(arrayDecl, dimensions);
	}
	else if (arrayDecl->isConst)
	{
		std::cerr << "Semantic Error: const array '" << arrayDecl->arrayName << "' must be initialized\n";
		hasError = true;
	}
	symbolTableStack.back()[arrayDecl->arrayName] =
		{ arrayDecl->elementType, arrayDecl->isConst, true, false, std::move(dimensions) };
}

void SemanticAnalyzer::validateArrayInitializer(ArrayDeclAST* arrayDecl, const std::vector<int>& dimensions)
{
	if (!arrayDecl->initVal)
	{
		return;
	}
	if (!dynamic_cast<InitListExprAST*>(arrayDecl->initVal.get()))
	{
		std::string initType = analyzeExpr(arrayDecl->initVal.get());
		if (!isAssignableType(arrayDecl->elementType, initType))
		{
			std::cerr << "Semantic Error: array '" << arrayDecl->arrayName
				<< "' initializer element type mismatch\n";
			hasError = true;
		}
		return;
	}

	size_t totalElements = 1;
	for (int dim : dimensions)
	{
		totalElements *= static_cast<size_t>(dim);
	}

	size_t linearIndex = 0;
	validateArrayInitializerRecursive(arrayDecl->initVal.get(), arrayDecl->elementType,
		dimensions, 0, linearIndex, totalElements, arrayDecl->arrayName);
}

void SemanticAnalyzer::validateArrayInitializerRecursive(ExprAST* init,
	const std::string& elementType,
	const std::vector<int>& dimensions,
	size_t depth,
	size_t& linearIndex,
	size_t totalElements,
	const std::string& arrayName)
{
	if (!init)
	{
		return;
	}

	if (auto* list = dynamic_cast<InitListExprAST*>(init))
	{
		size_t childBlockSize = 1;
		for (size_t i = depth + 1; i < dimensions.size(); ++i)
		{
			childBlockSize *= static_cast<size_t>(dimensions[i]);
		}

		for (const auto& element : list->elements)
		{
			if (linearIndex >= totalElements)
			{
				std::cerr << "Semantic Error: too many initializers for array '"
					<< arrayName << "'\n";
				hasError = true;
				return;
			}

			if (dynamic_cast<InitListExprAST*>(element.get()) && depth + 1 < dimensions.size())
			{
				size_t alignedStart = childBlockSize == 0 ? linearIndex
					: ((linearIndex + childBlockSize - 1) / childBlockSize) * childBlockSize;
				if (alignedStart >= totalElements)
				{
					std::cerr << "Semantic Error: too many initializers for array '"
						<< arrayName << "'\n";
					hasError = true;
					return;
				}
				linearIndex = alignedStart;
				validateArrayInitializerRecursive(element.get(), elementType, dimensions,
					depth + 1, linearIndex, totalElements, arrayName);
				if (linearIndex > alignedStart + childBlockSize)
				{
					std::cerr << "Semantic Error: too many initializers for array '"
						<< arrayName << "'\n";
					hasError = true;
					return;
				}
				linearIndex = alignedStart + childBlockSize;
			}
			else
			{
				validateArrayInitializerRecursive(element.get(), elementType, dimensions,
					dimensions.size(), linearIndex, totalElements, arrayName);
			}
		}
		return;
	}

	if (linearIndex >= totalElements)
	{
		std::cerr << "Semantic Error: too many initializers for array '"
			<< arrayName << "'\n";
		hasError = true;
		return;
	}

	std::string initType = analyzeExpr(init);
	if (!isAssignableType(elementType, initType))
	{
		std::cerr << "Semantic Error: array '" << arrayName
			<< "' initializer element type mismatch\n";
		hasError = true;
	}
	++linearIndex;
}

void SemanticAnalyzer::validateFunctionParameters(const std::vector<FunctionParamAST>& params)
{
	for (const auto& param : params)
	{
		for (const auto& dim : param.dimensions)
		{
			std::string dimType = analyzeExpr(dim.get());
			if (dimType != "unknown" && dimType != "int")
			{
				std::cerr << "Semantic Error: array parameter dimension must be int\n";
				hasError = true;
			}
			if (!isConstIntExpr(dim.get()))
			{
				std::cerr << "Semantic Error: array parameter dimension must be constant int expression\n";
				hasError = true;
			}
			int value = evalConstInt(dim.get());
			if (value <= 0)
			{
				std::cerr << "Semantic Error: array parameter dimension must be positive\n";
				hasError = true;
			}
		}
	}
}

std::string SemanticAnalyzer::analyzeVariableExpr(VariableExprAST* varExpr)
{
	const SymbolInfo* symbol = lookupSymbol(varExpr->name);
	if (!symbol)
	{
		std::cerr << "Semantic Error: Variable '" << varExpr->name << "' not declared\n";
		hasError = true;
		return "unknown";
	}
	return symbol->isArray ? symbol->type + "[]" : symbol->type;
}

std::string SemanticAnalyzer::analyzeArrayAccessExpr(ArrayAccessExprAST* arrayExpr)
{
	const SymbolInfo* symbol = lookupSymbol(arrayExpr->arrayName);
	if (!symbol)
	{
		std::cerr << "Semantic Error: Array '" << arrayExpr->arrayName << "' not declared\n";
		hasError = true;
		return "unknown";
	}
	if (!symbol->isArray)
	{
		std::cerr << "Semantic Error: '" << arrayExpr->arrayName << "' is not an array\n";
		hasError = true;
		return "unknown";
	}
	for (const auto& index : arrayExpr->indices)
	{
		std::string indexType = analyzeExpr(index.get());
		if (indexType != "unknown" && indexType != "int")
		{
			std::cerr << "Semantic Error: Array index must be int\n";
			hasError = true;
		}
	}
	size_t rank = symbol->dimensions.size() + (symbol->isArrayParam ? 1 : 0);
	if (arrayExpr->indices.size() > rank)
	{
		std::cerr << "Semantic Error: too many indices for array '" << arrayExpr->arrayName << "'\n";
		hasError = true;
		return "unknown";
	}
	return arrayExpr->indices.size() < rank ? symbol->type + "[]" : symbol->type;
}

void SemanticAnalyzer::analyzeFunctionDecl(FunctionDeclAST* funcDecl)
{
	enterScope();
	std::string oldReturnType = currentReturnType;
	currentReturnType = funcDecl->returnType;
	for (const auto& param : funcDecl->parameters)
	{
		if (symbolTableStack.back().count(param.name))
		{
			std::cerr << "Semantic Error: Parameter '" << param.name << "' redefinition\n";
			hasError = true;
		}
		std::vector<int> dimensions;
		for (const auto& dim : param.dimensions)
		{
			std::string dimType = analyzeExpr(dim.get());
			if (dimType != "unknown" && dimType != "int")
			{
				std::cerr << "Semantic Error: array parameter dimension must be int\n";
				hasError = true;
			}
			if (!isConstIntExpr(dim.get()))
			{
				std::cerr << "Semantic Error: array parameter dimension must be constant int expression\n";
				hasError = true;
			}
			int value = evalConstInt(dim.get());
			if (value <= 0)
			{
				std::cerr << "Semantic Error: array parameter dimension must be positive\n";
				hasError = true;
			}
			dimensions.push_back(value);
		}
		symbolTableStack.back()[param.name] =
			{ param.type, false, param.isArray, param.isArray, std::move(dimensions) };
	}
	if (funcDecl->body)
	{
		for (const auto& stmt : funcDecl->body->statements)
		{
			analyzeStmt(stmt.get());
		}
		if (funcDecl->returnType != "void" && !guaranteedReturns(funcDecl->body.get()))
		{
			std::cerr << "Semantic Error: non-void function '" << funcDecl->functionName
				<< "' may finish without returning a value\n";
			hasError = true;
		}
	}
	currentReturnType = oldReturnType;
	exitScope();
}

void SemanticAnalyzer::analyzeIfStmt(IfStmtAST* ifStmt)
{
	std::string condType = analyzeExpr(ifStmt->condition.get(), true);
	if (condType == "void" || isArrayType(condType))
	{
		std::cerr << "Semantic Error: invalid if condition\n";
		hasError = true;
	}
	if (ifStmt->thenBranch) analyzeStmt(ifStmt->thenBranch.get());
	if (ifStmt->elseBranch) analyzeStmt(ifStmt->elseBranch.get());
}

void SemanticAnalyzer::analyzeWhileStmt(WhileStmtAST* whileStmt)
{
	std::string condType = analyzeExpr(whileStmt->condition.get(), true);
	if (condType == "void" || isArrayType(condType))
	{
		std::cerr << "Semantic Error: invalid while condition\n";
		hasError = true;
	}
	if (whileStmt->body)
	{
		++loopDepth;
		analyzeStmt(whileStmt->body.get());
		--loopDepth;
	}
}

void SemanticAnalyzer::analyzeDoWhileStmt(DoWhileStmtAST* doWhileStmt)
{
	if (doWhileStmt->body)
	{
		++loopDepth;
		analyzeStmt(doWhileStmt->body.get());
		--loopDepth;
	}
	std::string condType = analyzeExpr(doWhileStmt->condition.get(), true);
	if (condType == "void" || isArrayType(condType))
	{
		std::cerr << "Semantic Error: invalid do-while condition\n";
		hasError = true;
	}
}

void SemanticAnalyzer::analyzeForStmt(ForStmtAST* forStmt)
{
	enterScope();
	if (forStmt->init) analyzeStmt(forStmt->init.get());
	if (forStmt->condition)
	{
		std::string condType = analyzeExpr(forStmt->condition.get(), true);
		if (condType == "void" || isArrayType(condType))
		{
			std::cerr << "Semantic Error: invalid for condition\n";
			hasError = true;
		}
	}
	if (forStmt->increment) analyzeStmt(forStmt->increment.get());
	if (forStmt->body)
	{
		++loopDepth;
		analyzeStmt(forStmt->body.get());
		--loopDepth;
	}
	exitScope();
}
