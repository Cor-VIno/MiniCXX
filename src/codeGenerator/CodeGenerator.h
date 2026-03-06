#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "AST.h" 

class CodeGenerator
{
private:
	// 生成的汇编代码字符串
    std::string assemblyCode;

    std::vector<std::string> stringPool; // 存所有的字符串

    int labelCount = 0;       // 用于生成唯一的控制流标签（如 .L_if_1, .L_while_2）
    int currentOffset = 0;    // 记录当前函数栈帧的局部变量总偏移量（比如分配了 -4, -8 等）
    std::string currentFunctionName;

    // 作用域栈：记录变量名到物理栈基址(rbp)相对偏移量的映射
    // 这与 SemanticAnalyzer 中的符号表栈非常相似
	// 比如在函数 foo 中，变量 a 的偏移量是 -4，变量 b 的偏移量是 -8，那么当前作用域的映射就是 { "a": -4, "b": -8 }
    std::vector<std::unordered_map<std::string, int>> stackOffsetMap;

public:
    CodeGenerator() = default;

    // 后端代码生成的核心入口
    void generate(ProgramAST* program);

    // 供外部获取最终生成的汇编代码
    std::string getAssembly() const
    {
        return assemblyCode;
    }

private:
    // --- 作用域与辅助函数 ---
    void enterScope();
    void exitScope();
    int getVarOffset(const std::string& varName);       // 向上查找变量的栈偏移量
    std::string createLabel(const std::string& prefix); // 生成全局唯一的汇编标签

    // --- 语句级别生成 (对应 StmtAST 的各种子类) ---
    void generateStmt(StmtAST* stmt);
    void generateFunctionDecl(FunctionDeclAST* funcDecl);
    void generateVarDecl(VarDeclStmtAST* varDecl);
    void generateReturnStmt(ReturnStmtAST* retStmt);
    void generateIfStmt(IfStmtAST* ifStmt);
    void generateWhileStmt(WhileStmtAST* whileStmt);
    void generateDoWhileStmt(DoWhileStmtAST* doWhileStmt);
    void generateBlockStmt(BlockStmtAST* blockStmt);

    // --- 表达式级别生成 (对应 ExprAST 的各种子类) ---
    // 约定：任何表达式计算完毕后，其最终结果必须存放在 eax 寄存器中
    void generateExpr(ExprAST* expr);
};