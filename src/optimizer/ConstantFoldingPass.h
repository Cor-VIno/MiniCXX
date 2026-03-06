#pragma once
#include "ASTPass.h"
#include <iostream>

class ConstantFoldingPass : public ASTPass {
public:
    // 覆盖基类的 run 方法
    std::unique_ptr<ProgramAST> run(std::unique_ptr<ProgramAST> program) override;

private:
    // 核心私有辅助函数：分别处理语句和表达式
    std::unique_ptr<StmtAST> optimizeStmt(std::unique_ptr<StmtAST> stmt);
    std::unique_ptr<ExprAST> optimizeExpr(std::unique_ptr<ExprAST> expr);
};