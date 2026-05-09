#include "ir/passes/SimplifyCFGPass.h"

#include "ir/constants/ConstantInt.h"
#include "ir/core/BasicBlock.h"
#include "ir/core/Function.h"
#include "ir/core/Instruction.h"
#include "ir/core/Module.h"

#include <algorithm>
#include <unordered_map>

bool SimplifyCFGPass::run(Module& module)
{
    removedBlockCount = 0;
    simplifiedBranchCount = 0;

    bool changed = false;
    changed |= simplifyConstantBranches(module);
    changed |= removeUnreachableBlocks(module);
    changed |= mergeSinglePredecessorBlocks(module);
    changed |= removeUnreachableBlocks(module);
    return changed;
}

bool SimplifyCFGPass::simplifyConstantBranches(Module& module)
{
    bool changed = false;

    for (auto* function : module.getFunctions())
    {
        if (!function)
        {
            continue;
        }
        for (auto* block : function->getBasicBlocks())
        {
            if (!block)
            {
                continue;
            }
            auto& instructions = block->getInstructions();
            if (instructions.empty())
            {
                continue;
            }

            auto last = std::prev(instructions.end());
            auto* branch = dynamic_cast<BranchInst*>(*last);
            if (!branch || !branch->isConditional())
            {
                continue;
            }

            auto* condition = dynamic_cast<ConstantInt*>(branch->getOperand(0));
            if (!condition)
            {
                continue;
            }

            BasicBlock* target = condition->getValue() != 0 ? branch->getTrueBlock() : branch->getFalseBlock();
            auto* replacement = new BranchInst(target);
            replacement->setParent(block);
            branch->dropAllOperands();
            *last = replacement;
            ++simplifiedBranchCount;
            changed = true;
        }
    }

    return changed;
}

bool SimplifyCFGPass::removeUnreachableBlocks(Module& module)
{
    bool changed = false;

    for (auto* function : module.getFunctions())
    {
        if (!function || function->getBasicBlocks().empty())
        {
            continue;
        }

        std::unordered_set<BasicBlock*> reachable;
        markReachable(function->getBasicBlocks().front(), reachable);

        auto& blocks = function->getBasicBlocks();
        for (auto it = blocks.begin(); it != blocks.end();)
        {
            BasicBlock* block = *it;
            if (reachable.find(block) == reachable.end())
            {
                for (auto* inst : block->getInstructions())
                {
                    if (inst)
                    {
                        inst->dropAllOperands();
                        inst->setParent(nullptr);
                    }
                }
                it = blocks.erase(it);
                ++removedBlockCount;
                changed = true;
                continue;
            }
            ++it;
        }
    }

    return changed;
}

bool SimplifyCFGPass::mergeSinglePredecessorBlocks(Module& module)
{
    bool changed = false;

    for (auto* function : module.getFunctions())
    {
        if (!function)
        {
            continue;
        }

        std::unordered_map<BasicBlock*, int> predecessorCount;
        for (auto* block : function->getBasicBlocks())
        {
            auto* branch = block ? dynamic_cast<BranchInst*>(block->getTerminator()) : nullptr;
            if (!branch)
            {
                continue;
            }
            ++predecessorCount[branch->getTrueBlock()];
            if (branch->isConditional())
            {
                ++predecessorCount[branch->getFalseBlock()];
            }
        }

        auto& blocks = function->getBasicBlocks();
        for (auto it = blocks.begin(); it != blocks.end(); ++it)
        {
            BasicBlock* block = *it;
            auto* branch = block ? dynamic_cast<BranchInst*>(block->getTerminator()) : nullptr;
            if (!branch || branch->isConditional())
            {
                continue;
            }

            BasicBlock* target = branch->getTrueBlock();
            if (!target || target == block || predecessorCount[target] != 1)
            {
                continue;
            }

            auto targetIt = std::find(blocks.begin(), blocks.end(), target);
            if (targetIt == blocks.end())
            {
                continue;
            }

            auto& blockInsts = block->getInstructions();
            if (!blockInsts.empty())
            {
                blockInsts.back()->dropAllOperands();
                blockInsts.back()->setParent(nullptr);
                blockInsts.pop_back();
            }

            auto targetInstructions = target->getInstructions();
            for (auto* inst : targetInstructions)
            {
                if (inst)
                {
                    inst->setParent(block);
                    blockInsts.push_back(inst);
                }
            }
            target->getInstructions().clear();
            blocks.erase(targetIt);
            changed = true;
            break;
        }
    }

    return changed;
}

void SimplifyCFGPass::markReachable(BasicBlock* block, std::unordered_set<BasicBlock*>& reachable) const
{
    if (!block || reachable.find(block) != reachable.end())
    {
        return;
    }
    reachable.insert(block);

    auto* branch = dynamic_cast<BranchInst*>(block->getTerminator());
    if (!branch)
    {
        return;
    }

    markReachable(branch->getTrueBlock(), reachable);
    if (branch->isConditional())
    {
        markReachable(branch->getFalseBlock(), reachable);
    }
}
