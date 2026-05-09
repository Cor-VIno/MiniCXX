#pragma once

// --- 1. 前向声明所有 AST 节点类 ---
// 这样就不需要在这里 #include "AST.h"，防止循环包含
class ProgramAST;

// 表达式类
class NumberExprAST;
class VariableExprAST;
class UnaryExprAST;
class BinaryExprAST;
class StringExprAST;
class CallExprAST;
class ArrayAccessExprAST;
class InitListExprAST;
class MemberAccessExprAST;

// 语句类
class ExprStmtAST;
class EmptyStmtAST;
class AssignStmtAST;
class BlockStmtAST;
class IfStmtAST;
class WhileStmtAST;
class DoWhileStmtAST;
class ForStmtAST; // 我们刚刚新增的
class ReturnStmtAST;
class BreakStmtAST;
class ContinueStmtAST;
class VarDeclStmtAST;
class DeclGroupAST;
class ArrayDeclAST;
class StructDeclAST;

// 函数类
class FunctionDeclAST;
class FunctionDeclStmtAST;

// 前向声明中端的基类
class Value;

// --- 2. 访问者接口类 ---
class ASTVisitor
{
public:
    virtual ~ASTVisitor() = default;

    // 每一个具体的 AST 节点都对应一个纯虚函数
    // 中端的 IRGenerator 将继承这个类并实现这些函数

    virtual Value* visitProgram(ProgramAST* node) = 0;

    // 表达式访问接口
    virtual Value* visitNumberExpr(NumberExprAST* node) = 0;
    virtual Value* visitVariableExpr(VariableExprAST* node) = 0;
    virtual Value* visitUnaryExpr(UnaryExprAST* node) = 0;
    virtual Value* visitBinaryExpr(BinaryExprAST* node) = 0;
    virtual Value* visitStringExpr(StringExprAST* node) = 0;
    virtual Value* visitCallExpr(CallExprAST* node) = 0;
    virtual Value* visitArrayAccessExpr(ArrayAccessExprAST* node) = 0;
    virtual Value* visitMemberAccessExpr(MemberAccessExprAST* node) = 0;
    virtual Value* visitInitListExpr(InitListExprAST* node) = 0;

    // 语句访问接口
    virtual Value* visitExprStmt(ExprStmtAST* node) = 0;
    virtual Value* visitEmptyStmt(EmptyStmtAST* node) = 0;
    virtual Value* visitAssignStmt(AssignStmtAST* node) = 0;
    virtual Value* visitBlockStmt(BlockStmtAST* node) = 0;
    virtual Value* visitIfStmt(IfStmtAST* node) = 0;
    virtual Value* visitWhileStmt(WhileStmtAST* node) = 0;
    virtual Value* visitDoWhileStmt(DoWhileStmtAST* node) = 0;
    virtual Value* visitForStmt(ForStmtAST* node) = 0; // 核心：处理 for 循环
    virtual Value* visitReturnStmt(ReturnStmtAST* node) = 0;
    virtual Value* visitBreakStmt(BreakStmtAST* node) = 0;
    virtual Value* visitContinueStmt(ContinueStmtAST* node) = 0;
    virtual Value* visitVarDeclStmt(VarDeclStmtAST* node) = 0;
    virtual Value* visitDeclGroup(DeclGroupAST* node) = 0;
    virtual Value* visitArrayDecl(ArrayDeclAST* node) = 0;
    virtual Value* visitStructDecl(StructDeclAST* node) = 0;

    // 函数访问接口
    virtual Value* visitFunctionDecl(FunctionDeclAST* node) = 0;
    virtual Value* visitFunctionDeclStmt(FunctionDeclStmtAST* node) = 0;
};
