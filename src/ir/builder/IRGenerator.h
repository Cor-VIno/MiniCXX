#pragma once

#include "ir/builder/ASTVisitor.h"
#include "ir/builder/IRBuilder.h"
#include "ir/core/Module.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class ArrayType;
class BasicBlock;
class Constant;
class ExprAST;
class Function;
struct FunctionParamAST;
class LValueAST;
class StmtAST;
class Type;

class IRGenerator : public ASTVisitor
{
private:
    struct Symbol
    {
        Value* address = nullptr;
        Type* valueType = nullptr;
        bool isArray = false;
        bool isArrayParam = false;
        std::vector<int> dimensions;
        Constant* constValue = nullptr;
    };

    struct FunctionInfo
    {
        Type* returnType = nullptr;
        std::vector<Type*> paramTypes;
        bool variadic = false;
    };

    Module module;
    IRBuilder builder;
    Function* currentFunction = nullptr;
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    std::unordered_map<std::string, FunctionInfo> functions;
    std::unordered_map<std::string, Constant*> constantValues;
    std::vector<BasicBlock*> breakTargets;
    std::vector<BasicBlock*> continueTargets;
    unsigned blockIndex = 0;
    unsigned stringIndex = 0;

public:
    IRGenerator();

    const Module& getModule() const
    {
        return module;
    }

    Module& getModule()
    {
        return module;
    }

    std::string printModule() const
    {
        return module.print();
    }

    Value* visitProgram(ProgramAST* node) override;
    Value* visitNumberExpr(NumberExprAST* node) override;
    Value* visitVariableExpr(VariableExprAST* node) override;
    Value* visitUnaryExpr(UnaryExprAST* node) override;
    Value* visitBinaryExpr(BinaryExprAST* node) override;
    Value* visitStringExpr(StringExprAST* node) override;
    Value* visitCallExpr(CallExprAST* node) override;
    Value* visitArrayAccessExpr(ArrayAccessExprAST* node) override;
    Value* visitMemberAccessExpr(MemberAccessExprAST* node) override;
    Value* visitInitListExpr(InitListExprAST* node) override;
    Value* visitExprStmt(ExprStmtAST* node) override;
    Value* visitEmptyStmt(EmptyStmtAST* node) override;
    Value* visitAssignStmt(AssignStmtAST* node) override;
    Value* visitBlockStmt(BlockStmtAST* node) override;
    Value* visitIfStmt(IfStmtAST* node) override;
    Value* visitWhileStmt(WhileStmtAST* node) override;
    Value* visitDoWhileStmt(DoWhileStmtAST* node) override;
    Value* visitForStmt(ForStmtAST* node) override;
    Value* visitReturnStmt(ReturnStmtAST* node) override;
    Value* visitBreakStmt(BreakStmtAST* node) override;
    Value* visitContinueStmt(ContinueStmtAST* node) override;
    Value* visitVarDeclStmt(VarDeclStmtAST* node) override;
    Value* visitDeclGroup(DeclGroupAST* node) override;
    Value* visitArrayDecl(ArrayDeclAST* node) override;
    Value* visitStructDecl(StructDeclAST* node) override;
    Value* visitFunctionDecl(FunctionDeclAST* node) override;
    Value* visitFunctionDeclStmt(FunctionDeclStmtAST* node) override;

private:
    void enterScope();
    void exitScope();
    void declareBuiltins();
    void collectGlobalConstant(StmtAST* stmt);
    Symbol* lookupSymbol(const std::string& name);
    BasicBlock* createBlock(const std::string& prefix);
    void ensureBranchTo(BasicBlock* target);
    Type* typeFromName(const std::string& name) const;
    Type* arrayTypeFromDimensions(Type* elementType, const std::vector<int>& dimensions) const;
    Type* arrayParamStorageType(const FunctionParamAST& param) const;
    Type* elementTypeAfterIndices(Type* aggregateType, size_t indexCount) const;
    Value* emitStringLiteral(const std::string& value);
    Constant* lookupConstantValue(const std::string& name) const;
    int evaluateConstantIntDimension(ExprAST* expr) const;
    Constant* evaluateConstantInitializer(ExprAST* expr, Type* targetType) const;
    Constant* evaluateConstantArrayInitializer(ExprAST* init, Type* arrayType, Type* scalarType,
        const std::vector<int>& dimensions) const;
    void collectArrayConstantInitializers(ExprAST* init, Type* scalarType, const std::vector<int>& dimensions,
        std::vector<Constant*>& flatValues, size_t depth, size_t& linearIndex) const;
    Constant* buildConstantArray(Type* aggregateType, Type* scalarType, const std::vector<int>& dimensions,
        const std::vector<Constant*>& flatValues, size_t depth, size_t& offset) const;
    Value* defaultValue(Type* type) const;
    Value* castTo(Value* value, Type* targetType);
    Value* castToCondition(Value* value);
    Value* emitLValueAddress(LValueAST* lvalue, Type** elementType = nullptr);
    Value* emitArrayElementAddress(const Symbol& symbol, const std::vector<std::unique_ptr<ExprAST>>& indices,
        Type** elementType = nullptr);
    void emitArrayInitStores(const Symbol& symbol, ExprAST* init, Type* scalarType);
    void emitArrayInitStoresRecursive(const Symbol& symbol, ExprAST* init, Type* scalarType,
        size_t depth, size_t& linearIndex);
    void emitArrayScalarStore(const Symbol& symbol, ExprAST* init, Type* scalarType, size_t linearIndex);
    FunctionInfo makeFunctionInfo(const std::string& returnType, const std::vector<FunctionParamAST>& params) const;
    void registerFunction(const std::string& name, const FunctionInfo& info);
};
