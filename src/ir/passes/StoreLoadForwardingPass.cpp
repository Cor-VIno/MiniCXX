#include "ir/passes/StoreLoadForwardingPass.h"

#include "ir/core/BasicBlock.h"
#include "ir/core/Function.h"
#include "ir/core/Instruction.h"
#include "ir/core/Module.h"

#include <cstdint>
#include <sstream>
#include <unordered_map>

struct KnownMemoryValue
{
    Value* ptr = nullptr;
    Value* value = nullptr;
};

static bool isLocalScalarAlloca(Value* value)
{
    auto* alloca = dynamic_cast<AllocaInst*>(value);
    return alloca && !alloca->getAllocatedType()->isArrayTy();
}

static std::string valueIdentity(Value* value)
{
    std::ostringstream os;
    os << reinterpret_cast<std::uintptr_t>(value);
    return os.str();
}

static std::string memoryKey(Value* ptr)
{
    if (auto* gep = dynamic_cast<GEPInst*>(ptr))
    {
        std::string key = "gep(" + valueIdentity(gep->getOperand(0));
        for (size_t i = 1; i < gep->getNumOperands(); ++i)
        {
            key += ",";
            key += gep->getOperand(i)->operandString();
        }
        key += ")";
        return key;
    }
    return "ptr(" + valueIdentity(ptr) + ")";
}

bool StoreLoadForwardingPass::run(Module& module)
{
    forwardedLoadCount = 0;
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

            std::unordered_map<std::string, KnownMemoryValue> knownMemoryValue;
            auto& instructions = block->getInstructions();
            for (auto it = instructions.begin(); it != instructions.end();)
            {
                Instruction* inst = *it;
                if (auto* load = dynamic_cast<LoadInst*>(inst))
                {
                    Value* ptr = load->getOperand(0);
                    auto found = knownMemoryValue.find(memoryKey(ptr));
                    if (found != knownMemoryValue.end())
                    {
                        load->replaceAllUsesWith(found->second.value);
                        load->dropAllOperands();
                        load->setParent(nullptr);
                        it = instructions.erase(it);
                        ++forwardedLoadCount;
                        changed = true;
                        continue;
                    }
                }
                else if (dynamic_cast<StoreInst*>(inst))
                {
                    knownMemoryValue[memoryKey(inst->getOperand(1))] = { inst->getOperand(1), inst->getOperand(0) };
                }
                else if (dynamic_cast<CallInst*>(inst))
                {
                    for (auto itKnown = knownMemoryValue.begin(); itKnown != knownMemoryValue.end();)
                    {
                        if (!isLocalScalarAlloca(itKnown->second.ptr))
                        {
                            itKnown = knownMemoryValue.erase(itKnown);
                            continue;
                        }
                        ++itKnown;
                    }
                }

                ++it;
            }
        }
    }

    return changed;
}
