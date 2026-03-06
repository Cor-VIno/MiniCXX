#include "ConstantFoldingPass.h"
#include <string>

/*
这块太麻烦了就直接ai跑了
核心思路就是一个语句优化一个表达式优化

语句优化就是把语句内部的所有表达式折叠,递归调用一下就行

表达式优化就是看当前是不是二元运算
如果是二元运算就先递归优化左右子树,然后看左右子树是不是数字
如果是数字就直接计算结果,返回一个新的数字节点
当然肯定要判断左右数字是不是一个类型,但是我现在只有int字面值,且其实NumberExprAST里面应该记录number的类型,但是我现在就直接当成int了,所以就不判断了
*/




// ==========================================
// 1. 顶层入口：优化整个程序
// ==========================================
std::unique_ptr<ProgramAST> ConstantFoldingPass::run(std::unique_ptr<ProgramAST> program)
{
    if (!program) return nullptr;

    std::cout << "  -> Running Constant Folding Pass...\n";

    // 遍历 ProgramAST 中的所有顶层声明 [1]
    for (auto& decl : program->declarations)
    {
        decl = optimizeStmt(std::move(decl));
    }

    return program;
}

// ==========================================
// 2. 表达式优化：常量折叠的核心魔法
// ==========================================
std::unique_ptr<ExprAST> ConstantFoldingPass::optimizeExpr(std::unique_ptr<ExprAST> expr)
{
    if (!expr) return nullptr;

    // A. 处理二元运算 (BinaryExprAST) [1]
    if (auto* binExpr = dynamic_cast<BinaryExprAST*>(expr.get()))
    {

        // 1. 后序遍历：先递归折叠左子树和右子树
        binExpr->LHS = optimizeExpr(std::move(binExpr->LHS));
        binExpr->RHS = optimizeExpr(std::move(binExpr->RHS));

        // 2. 检查左右子树是否都变成了单纯的数字 (NumberExprAST) [1]
        auto* leftNum = dynamic_cast<NumberExprAST*>(binExpr->LHS.get());
        auto* rightNum = dynamic_cast<NumberExprAST*>(binExpr->RHS.get());

        if (leftNum && rightNum)
        {
            // 提取字符串并转换为 C++ 的整数
            int leftVal = std::stoi(leftNum->value);
            int rightVal = std::stoi(rightNum->value);
            int result = 0;

            // 3. 执行常量计算
            if (binExpr->op == "+") result = leftVal + rightVal;
            else if (binExpr->op == "-") result = leftVal - rightVal;
            else if (binExpr->op == "*") result = leftVal * rightVal;
            else if (binExpr->op == "/")
            {
                if (rightVal == 0)
                {
                    std::cerr << "Optimizer Warning: Division by zero detected. Skipping fold.\n";
                    return expr; // 除数为0，放弃折叠，保留原样
                }
                result = leftVal / rightVal;
            }
            // 支持关系运算符折叠
            else if (binExpr->op == "==") result = (leftVal == rightVal);
            else if (binExpr->op == "!=") result = (leftVal != rightVal);
            else if (binExpr->op == "<") result = (leftVal < rightVal);
            else if (binExpr->op == "<=") result = (leftVal <= rightVal);
            else if (binExpr->op == ">") result = (leftVal > rightVal);
            else if (binExpr->op == ">=") result = (leftVal >= rightVal);
            else
            {
                return expr; // 未知运算符，放弃折叠
            }

            // 4. 见证奇迹：丢弃旧的 BinaryExprAST，返回全新的 NumberExprAST
            return std::make_unique<NumberExprAST>(std::to_string(result));
        }

        return expr; // 如果左右不全是数字，无法折叠，原样返回
    }

    // B. 处理函数调用 (CallExprAST) [1]
    else if (auto* callExpr = dynamic_cast<CallExprAST*>(expr.get()))
    {
        // 递归优化所有的传入参数
        for (auto& arg : callExpr->args)
        {
            arg = optimizeExpr(std::move(arg));
        }
        return expr;
    }

    // 对于普通的 NumberExprAST 和 VariableExprAST，无需优化，直接返回 [1]
    return expr;
}

// ==========================================
// 3. 语句优化：负责桥接和递归遍历树结构
// ==========================================
std::unique_ptr<StmtAST> ConstantFoldingPass::optimizeStmt(std::unique_ptr<StmtAST> stmt)
{
    if (!stmt) return nullptr;

    // A. 变量声明 (VarDeclStmtAST) [1]
    if (auto* varDecl = dynamic_cast<VarDeclStmtAST*>(stmt.get()))
    {
        if (varDecl->initValue)
        {
            varDecl->initValue = optimizeExpr(std::move(varDecl->initValue));
        }
        return stmt;
    }

    // B. 独立表达式语句 (ExprStmtAST) [1]
    else if (auto* exprStmt = dynamic_cast<ExprStmtAST*>(stmt.get()))
    {
        exprStmt->expr = optimizeExpr(std::move(exprStmt->expr));
        return stmt;
    }

    // C. Return 语句 (ReturnStmtAST) [1]
    else if (auto* retStmt = dynamic_cast<ReturnStmtAST*>(stmt.get()))
    {
        if (retStmt->returnValue)
        {
            retStmt->returnValue = optimizeExpr(std::move(retStmt->returnValue));
        }
        return stmt;
    }

    // D. If 条件语句 (IfStmtAST) [1]
    else if (auto* ifStmt = dynamic_cast<IfStmtAST*>(stmt.get()))
    {
        ifStmt->condition = optimizeExpr(std::move(ifStmt->condition));
        ifStmt->thenBranch = optimizeStmt(std::move(ifStmt->thenBranch));
        if (ifStmt->elseBranch)
        {
            ifStmt->elseBranch = optimizeStmt(std::move(ifStmt->elseBranch));
        }
        return stmt;
    }

    // E. While 循环语句 (WhileStmtAST) [1]
    else if (auto* whileStmt = dynamic_cast<WhileStmtAST*>(stmt.get()))
    {
        whileStmt->condition = optimizeExpr(std::move(whileStmt->condition));
        whileStmt->body = optimizeStmt(std::move(whileStmt->body));
        return stmt;
    }

    // F. Do-While 循环语句 (DoWhileStmtAST) [1]
    else if (auto* doWhileStmt = dynamic_cast<DoWhileStmtAST*>(stmt.get()))
    {
        doWhileStmt->body = optimizeStmt(std::move(doWhileStmt->body));
        doWhileStmt->condition = optimizeExpr(std::move(doWhileStmt->condition));
        return stmt;
    }

    // G. 代码块 (BlockStmtAST) [1]
    else if (auto* blockStmt = dynamic_cast<BlockStmtAST*>(stmt.get()))
    {
        for (auto& s : blockStmt->statements)
        {
            s = optimizeStmt(std::move(s));
        }
        return stmt;
    }

    // H. 函数定义 (FunctionDeclAST) [1]
    else if (auto* funcDecl = dynamic_cast<FunctionDeclAST*>(stmt.get()))
    {
        // 注意：funcDecl 的 body 是 std::unique_ptr<BlockStmtAST> [1]
        // 所以我们直接遍历它内部的 statements 即可
        if (funcDecl->body)
        {
            for (auto& s : funcDecl->body->statements)
            {
                s = optimizeStmt(std::move(s));
            }
        }
        return stmt;
    }

    // 其他语句类型（如 FunctionDeclStmtAST 无函数体等）原样返回
    return stmt;
}