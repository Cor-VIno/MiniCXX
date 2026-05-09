#pragma once

#include "frontend/ast/AST.h"

#include <string>
#include <unordered_map>
#include <vector>

class SemanticAnalyzer
{
private:
	struct SymbolInfo
	{
		std::string type;
		bool isConst = false;
		bool isArray = false;
		bool isArrayParam = false;
		std::vector<int> dimensions;
		bool hasConstIntValue = false;
		int constIntValue = 0;
	};

	struct FunctionSignature
	{
		std::string returnType;
		std::vector<std::string> paramTypes;
		std::vector<bool> paramIsArray;
		std::vector<std::vector<int>> paramDimensions;
		bool builtin = false;
		bool isDefinition = false;
	};

	std::vector<std::unordered_map<std::string, SymbolInfo>> symbolTableStack;
	std::unordered_map<std::string, std::vector<FunctionSignature>> functions;
	std::unordered_map<std::string, int> knownConstInts;
	bool hasMainFunction = false;
	bool hasError = false;
	int loopDepth = 0;
	std::string currentReturnType;

public:
	bool analyze(ProgramAST* root);

private:
	void enterScope();
	void exitScope();

	void registerBuiltinFunctions();
	void collectGlobalConstInt(StmtAST* stmt);
	void collectFunctionSignature(StmtAST* stmt);
	void addFunctionSignature(const std::string& name, const FunctionSignature& signature);
	const FunctionSignature* findFunction(const std::string& name, const std::vector<std::string>& argTypes) const;
	const FunctionSignature* findFunction(const std::string& name,
		const std::vector<std::string>& argTypes,
		const std::vector<std::vector<int>>& argArrayDimensions) const;
	bool isFunctionCompatible(const FunctionSignature& signature,
		const std::vector<std::string>& argTypes,
		const std::vector<std::vector<int>>& argArrayDimensions) const;

	std::string analyzeExpr(ExprAST* expr, bool allowConditionOps = false);
	void analyzeStmt(StmtAST* stmt);

	void analyzeVarDecl(VarDeclStmtAST* varDecl);
	void analyzeArrayDecl(ArrayDeclAST* arrayDecl);
	void validateArrayInitializer(ArrayDeclAST* arrayDecl, const std::vector<int>& dimensions);
	void validateArrayInitializerRecursive(ExprAST* init,
		const std::string& elementType,
		const std::vector<int>& dimensions,
		size_t depth,
		size_t& linearIndex,
		size_t totalElements,
		const std::string& arrayName);
	void validateFunctionParameters(const std::vector<FunctionParamAST>& params);
	std::string analyzeVariableExpr(VariableExprAST* varExpr);
	std::string analyzeArrayAccessExpr(ArrayAccessExprAST* arrayExpr);
	void analyzeFunctionDecl(FunctionDeclAST* funcDecl);
	void analyzeIfStmt(IfStmtAST* ifStmt);
	void analyzeWhileStmt(WhileStmtAST* whileStmt);
	void analyzeDoWhileStmt(DoWhileStmtAST* doWhileStmt);
	void analyzeForStmt(ForStmtAST* forStmt);

	SymbolInfo* lookupSymbol(const std::string& name);
	const SymbolInfo* lookupSymbol(const std::string& name) const;
	bool isTopLevelContext() const;
	bool guaranteedReturns(StmtAST* stmt) const;
	bool isConstLValue(LValueAST* lvalue) const;
	int evalConstInt(ExprAST* expr) const;
	bool isArrayType(const std::string& type) const;
	std::string baseTypeOf(const std::string& type) const;
	bool isAssignableType(const std::string& left, const std::string& right) const;
	bool isNumericType(const std::string& type) const;
	bool isConditionOperator(const std::string& op) const;
	std::string usualArithmeticType(const std::string& left, const std::string& right) const;
	std::vector<int> decayedArrayDimensions(ExprAST* expr) const;
	bool isConstIntExpr(ExprAST* expr) const;
};
