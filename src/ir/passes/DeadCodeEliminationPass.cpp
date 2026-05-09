#include "ir/passes/DeadCodeEliminationPass.h"

#include "ir/core/BasicBlock.h"
#include "ir/core/Function.h"
#include "ir/core/Instruction.h"
#include "ir/core/Module.h"

bool DeadCodeEliminationPass::run(Module& module)
{
    removedInstructionCount = 0;
    bool changed = false;
    bool localChanged = true;

    while (localChanged)
    {
        localChanged = false;
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
                for (auto it = instructions.begin(); it != instructions.end();)
                {
                    Instruction* inst = *it;
                    if (isRemovable(inst) && inst->useEmpty())
                    {
                        inst->dropAllOperands();
                        inst->setParent(nullptr);
                        it = instructions.erase(it);
                        ++removedInstructionCount;
                        changed = true;
                        localChanged = true;
                        continue;
                    }
                    ++it;
                }
            }
        }
    }

    return changed;
}

bool DeadCodeEliminationPass::isRemovable(Instruction* inst) const
{
    if (!inst)
    {
        return false;
    }

    switch (inst->getOpcode())
    {
    case Opcode::Alloca:
    case Opcode::Load:
    case Opcode::Add:
    case Opcode::Sub:
    case Opcode::Mul:
    case Opcode::SDiv:
    case Opcode::SRem:
    case Opcode::FAdd:
    case Opcode::FSub:
    case Opcode::FMul:
    case Opcode::FDiv:
    case Opcode::ICmp:
    case Opcode::FCmp:
    case Opcode::GEP:
    case Opcode::Cast:
    case Opcode::Phi:
    case Opcode::Select:
    case Opcode::InsertElement:
    case Opcode::ExtractElement:
        return true;
    default:
        return false;
    }
}
