#include "frontend/ast/AST.h"
#include "ir/builder/ASTVisitor.h" // 确保路径正确指向你的 Visitor 接口

// --- 表达式类实现 ---

Value* NumberExprAST::accept(ASTVisitor& visitor)
{
    return visitor.visitNumberExpr(this);
}

Value* VariableExprAST::accept(ASTVisitor& visitor)
{
    return visitor.visitVariableExpr(this);
}

Value* BinaryExprAST::accept(ASTVisitor& visitor)
{
    return visitor.visitBinaryExpr(this);
}

Value* UnaryExprAST::accept(ASTVisitor& visitor)
{
    return visitor.visitUnaryExpr(this);
}

Value* StringExprAST::accept(ASTVisitor& visitor)
{
    return visitor.visitStringExpr(this);
}

Value* CallExprAST::accept(ASTVisitor& visitor)
{
    return visitor.visitCallExpr(this);
}

Value* ArrayAccessExprAST::accept(ASTVisitor& visitor)
{
    return visitor.visitArrayAccessExpr(this);
}

Value* MemberAccessExprAST::accept(ASTVisitor& visitor)
{
    return visitor.visitMemberAccessExpr(this);
}

// --- 语句类实现 ---

Value* ExprStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitExprStmt(this);
}

Value* EmptyStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitEmptyStmt(this);
}

Value* AssignStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitAssignStmt(this);
}

Value* BlockStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitBlockStmt(this);
}

Value* IfStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitIfStmt(this);
}

Value* WhileStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitWhileStmt(this);
}

Value* DoWhileStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitDoWhileStmt(this);
}

Value* ForStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitForStmt(this);
}

Value* ReturnStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitReturnStmt(this);
}

Value* BreakStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitBreakStmt(this);
}

Value* ContinueStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitContinueStmt(this);
}

Value* VarDeclStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitVarDeclStmt(this);
}

Value* DeclGroupAST::accept(ASTVisitor& visitor)
{
    return visitor.visitDeclGroup(this);
}

Value* ArrayDeclAST::accept(ASTVisitor& visitor)
{
    return visitor.visitArrayDecl(this);
}


Value* InitListExprAST::accept(ASTVisitor& visitor)
{
    return visitor.visitInitListExpr(this);
}

Value* StructDeclAST::accept(ASTVisitor& visitor)
{
    return visitor.visitStructDecl(this);
}

// --- 顶层类实现 ---

Value* ProgramAST::accept(ASTVisitor& visitor)
{
    return visitor.visitProgram(this);
}

Value* FunctionDeclAST::accept(ASTVisitor& visitor)
{
    return visitor.visitFunctionDecl(this);
}

Value* FunctionDeclStmtAST::accept(ASTVisitor& visitor)
{
    return visitor.visitFunctionDeclStmt(this);
}
