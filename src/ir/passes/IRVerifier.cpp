#include "ir/passes/IRVerifier.h"

#include "ir/core/BasicBlock.h"
#include "ir/core/Function.h"
#include "ir/core/Instruction.h"
#include "ir/core/Module.h"

#include <iostream>
#include <unordered_set>

bool IRVerifier::verify(const Module& module)
{
    errors.clear();
    for (const auto* function : module.getFunctions())
    {
        if (!function || function->getBasicBlocks().empty())
        {
            continue;
        }

        std::unordered_set<const BasicBlock*> blocksInFunction;
        for (const auto* block : function->getBasicBlocks())
        {
            if (block)
            {
                blocksInFunction.insert(block);
                if (block->getParent() != function)
                {
                    errors.push_back("basic block %" + block->getName()
                        + " has mismatched parent in function @" + function->getName());
                }
            }
        }

        for (const auto* block : function->getBasicBlocks())
        {
            if (!block)
            {
                errors.push_back("null basic block in function @" + function->getName());
                continue;
            }
            if (!block->getTerminator())
            {
                errors.push_back("basic block %" + block->getName() + " in function @"
                    + function->getName() + " has no terminator");
            }
            bool sawTerminator = false;
            for (const auto* inst : block->getInstructions())
            {
                if (!inst)
                {
                    errors.push_back("null instruction in block %" + block->getName());
                    continue;
                }
                if (inst->getParent() != block)
                {
                    errors.push_back("instruction parent mismatch in block %" + block->getName());
                }
                for (size_t i = 0; i < inst->getNumOperands(); ++i)
                {
                    if (!inst->getOperand(i))
                    {
                        errors.push_back("null operand in instruction in block %" + block->getName());
                    }
                }
                if (sawTerminator)
                {
                    errors.push_back("instruction after terminator in block %" + block->getName());
                    break;
                }
                if (inst && inst->isTerminator())
                {
                    sawTerminator = true;
                }

                if (auto* branch = dynamic_cast<const BranchInst*>(inst))
                {
                    if (!branch->getTrueBlock()
                        || blocksInFunction.find(branch->getTrueBlock()) == blocksInFunction.end())
                    {
                        errors.push_back("branch in block %" + block->getName()
                            + " targets a block outside function @" + function->getName());
                    }
                    if (branch->isConditional()
                        && (!branch->getFalseBlock()
                            || blocksInFunction.find(branch->getFalseBlock()) == blocksInFunction.end()))
                    {
                        errors.push_back("conditional branch in block %" + block->getName()
                            + " has invalid false target");
                    }
                }
            }
        }
    }
    return errors.empty();
}

void IRVerifier::printErrors() const
{
    for (const auto& error : errors)
    {
        std::cerr << "IR Verifier Error: " << error << "\n";
    }
}
