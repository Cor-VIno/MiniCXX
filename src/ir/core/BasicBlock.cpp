#include "ir/core/BasicBlock.h"

#include "ir/core/Function.h"
#include "ir/core/Instruction.h"

#include <algorithm>
#include <sstream>

void BasicBlock::addInstruction(Instruction* inst)
{
    if (!inst)
    {
        return;
    }
    inst->setParent(this);
    instList.push_back(inst);
}

void BasicBlock::removeInstruction(Instruction* inst)
{
    instList.erase(std::remove(instList.begin(), instList.end(), inst), instList.end());
    if (inst)
    {
        inst->setParent(nullptr);
    }
}

Instruction* BasicBlock::getTerminator() const
{
    if (instList.empty())
    {
        return nullptr;
    }
    Instruction* last = instList.back();
    return last && last->isTerminator() ? last : nullptr;
}

std::string BasicBlock::print() const
{
    std::ostringstream os;
    os << name << ":\n";
    for (const auto* inst : instList)
    {
        if (inst)
        {
            os << "  " << inst->print() << "\n";
        }
    }
    return os.str();
}
