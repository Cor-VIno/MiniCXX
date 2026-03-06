#pragma once
#include <memory>
#include "AST.h"


/*
当你新建优化类的时候
你需要先在PassManager导入头文件
然后在PassManager的buildPipeline函数中根据优化级别添加你的优化类
*/



// ASTPass 是所有 AST 优化趟的基类
class ASTPass
{
public:
    virtual ~ASTPass() = default;

    // 核心接口：接收一个旧的 ProgramAST，返回一个优化后的 ProgramAST
    // 因为 AST 节点都是由 unique_ptr 管理的，所以必须按值传递并转移所有权
    virtual std::unique_ptr<ProgramAST> run(std::unique_ptr<ProgramAST> program) = 0;
};