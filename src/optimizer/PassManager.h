#pragma once
#include <vector>
#include <memory>
#include "ASTPass.h"

// 实在没招了,我不想全部优化类都在main中实例,但是又想有优化力度选择,所以就做了这个PassManager,根据传入的优化级别来组装优化流水线
// 把所有的优化类包含进来



/*
当你新建优化类的时候
你需要先导入头文件
然后在PassManager的buildPipeline函数中根据优化级别添加你的优化类
*/





#include "ConstantFoldingPass.h"
// #include "DeadCodeEliminationPass.h"
// #include "LoopUnrollPass.h"

class PassManager
{
private:
    std::vector<std::unique_ptr<ASTPass>> passes;

public:
    PassManager(int optLevel = 0)
    {
        buildPipeline(optLevel);
    }

    std::unique_ptr<ProgramAST> runAll(std::unique_ptr<ProgramAST> program)
    {
        for (auto& pass : passes)
        {
            program = pass->run(std::move(program));
        }
        return program;
    }

private:
    void buildPipeline(int optLevel)
    {
        // optLevel == 0 (如 gcc -O0): 不添加任何 Pass，直接结束
        if (optLevel == 0) return;

        // optLevel >= 1 (如 gcc -O1): 添加基础优化
        if (optLevel >= 1)
        {
            passes.push_back(std::make_unique<ConstantFoldingPass>());
        }

        // optLevel >= 2 (如 gcc -O2): 添加进阶优化
        if (optLevel >= 2)
        {
            // passes.push_back(std::make_unique<DeadCodeEliminationPass>());
        }

        // optLevel >= 3 (如 gcc -O3): 添加激进优化
        if (optLevel >= 3)
        {
            // passes.push_back(std::make_unique<LoopUnrollPass>());
        }
    }
};