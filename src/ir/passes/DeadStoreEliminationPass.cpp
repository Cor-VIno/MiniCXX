#include "ir/passes/DeadStoreEliminationPass.h"

#include "ir/core/BasicBlock.h"
#include "ir/core/Function.h"
#include "ir/core/Instruction.h"
#include "ir/core/Module.h"

#include <unordered_set>

static bool isLocalScalarAlloca(Value* value)
{
    auto* alloca = dynamic_cast<AllocaInst*>(value);
    return alloca && !alloca->getAllocatedType()->isArrayTy();
}

bool DeadStoreEliminationPass::run(Module& module)
{
    removedStoreCount = 0;
    bool changed = false;

    for (auto* function : module.getFunctions())
    {
        if (!function)
        {
            continue;
        }

        for (auto* block : function->getBasicBlocks())
        {
            if (!block || !dynamic_cast<ReturnInst*>(block->getTerminator()))
            {
                continue;
            }

            std::unordered_set<Value*> livePointers;
            auto& instructions = block->getInstructions();
            for (auto it = instructions.end(); it != instructions.begin();)
            {
                --it;
                Instruction* inst = *it;

                if (auto* load = dynamic_cast<LoadInst*>(inst))
                {
                    livePointers.insert(load->getOperand(0));
                    continue;
                }

                if (dynamic_cast<CallInst*>(inst))
                {
                    for (auto* operand : inst->getOperands())
                    {
                        if (isLocalScalarAlloca(operand))
                        {
                            livePointers.insert(operand);
                        }
                    }
                    continue;
                }

                if (auto* store = dynamic_cast<StoreInst*>(inst))
                {
                    Value* ptr = store->getOperand(1);
                    if (isLocalScalarAlloca(ptr) && livePointers.find(ptr) == livePointers.end())
                    {
                        store->dropAllOperands();
                        store->setParent(nullptr);
                        it = instructions.erase(it);
                        ++removedStoreCount;
                        changed = true;
                        continue;
                    }
                    livePointers.erase(ptr);
                }
            }
        }
    }

    return changed;
}
