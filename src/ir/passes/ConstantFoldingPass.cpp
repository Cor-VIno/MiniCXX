#include "ir/passes/ConstantFoldingPass.h"

#include "ir/constants/ConstantFloat.h"
#include "ir/constants/ConstantInt.h"
#include "ir/core/BasicBlock.h"
#include "ir/core/Function.h"
#include "ir/core/Instruction.h"
#include "ir/core/Module.h"

#include <cmath>

bool ConstantFoldingPass::run(Module& module)
{
    foldedCount = 0;
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
            for (auto it = instructions.begin(); it != instructions.end();)
            {
                Instruction* inst = *it;
                Value* folded = foldInstruction(inst);
                if (folded && folded != inst)
                {
                    inst->replaceAllUsesWith(folded);
                    inst->dropAllOperands();
                    inst->setParent(nullptr);
                    it = instructions.erase(it);
                    ++foldedCount;
                    changed = true;
                    continue;
                }
                ++it;
            }
        }
    }

    return changed;
}

Value* ConstantFoldingPass::foldInstruction(Instruction* inst) const
{
    if (!inst)
    {
        return nullptr;
    }

    switch (inst->getOpcode())
    {
    case Opcode::Add:
    case Opcode::Sub:
    case Opcode::Mul:
    case Opcode::SDiv:
    case Opcode::SRem:
    case Opcode::FAdd:
    case Opcode::FSub:
    case Opcode::FMul:
    case Opcode::FDiv:
        return foldBinary(inst);
    case Opcode::ICmp:
        return foldICmp(inst);
    case Opcode::FCmp:
        return foldFCmp(inst);
    case Opcode::Cast:
        return foldCast(inst);
    default:
        return nullptr;
    }
}

Value* ConstantFoldingPass::foldBinary(Instruction* inst) const
{
    Value* lhs = inst->getOperand(0);
    Value* rhs = inst->getOperand(1);

    auto* lhsInt = dynamic_cast<ConstantInt*>(lhs);
    auto* rhsInt = dynamic_cast<ConstantInt*>(rhs);
    if (lhsInt && rhsInt)
    {
        int l = lhsInt->getValue();
        int r = rhsInt->getValue();
        switch (inst->getOpcode())
        {
        case Opcode::Add: return ConstantInt::get(l + r);
        case Opcode::Sub: return ConstantInt::get(l - r);
        case Opcode::Mul: return ConstantInt::get(l * r);
        case Opcode::SDiv: return r == 0 ? nullptr : ConstantInt::get(l / r);
        case Opcode::SRem: return r == 0 ? nullptr : ConstantInt::get(l % r);
        default: break;
        }
    }

    auto* lhsFloat = dynamic_cast<ConstantFloat*>(lhs);
    auto* rhsFloat = dynamic_cast<ConstantFloat*>(rhs);
    if (lhsFloat && rhsFloat)
    {
        float l = lhsFloat->getValue();
        float r = rhsFloat->getValue();
        switch (inst->getOpcode())
        {
        case Opcode::FAdd: return ConstantFloat::get(l + r);
        case Opcode::FSub: return ConstantFloat::get(l - r);
        case Opcode::FMul: return ConstantFloat::get(l * r);
        case Opcode::FDiv: return r == 0.0f ? nullptr : ConstantFloat::get(l / r);
        default: break;
        }
    }

    if (rhsInt)
    {
        int r = rhsInt->getValue();
        if ((inst->getOpcode() == Opcode::Add || inst->getOpcode() == Opcode::Sub) && r == 0)
        {
            return lhs;
        }
        if (inst->getOpcode() == Opcode::Mul && r == 1)
        {
            return lhs;
        }
        if (inst->getOpcode() == Opcode::Mul && r == 0)
        {
            return ConstantInt::get(0);
        }
        if (inst->getOpcode() == Opcode::SDiv && r == 1)
        {
            return lhs;
        }
        if (inst->getOpcode() == Opcode::SRem && r == 1)
        {
            return ConstantInt::get(0);
        }
    }

    if (lhsInt)
    {
        int l = lhsInt->getValue();
        if (inst->getOpcode() == Opcode::Add && l == 0)
        {
            return rhs;
        }
        if (inst->getOpcode() == Opcode::Mul && l == 1)
        {
            return rhs;
        }
        if (inst->getOpcode() == Opcode::Mul && l == 0)
        {
            return ConstantInt::get(0);
        }
    }

    return nullptr;
}

Value* ConstantFoldingPass::foldICmp(Instruction* inst) const
{
    auto* cmp = dynamic_cast<ICmpInst*>(inst);
    if (!cmp)
    {
        return nullptr;
    }

    auto* rhsZero = dynamic_cast<ConstantInt*>(inst->getOperand(1));
    auto* lhsCast = dynamic_cast<CastInst*>(inst->getOperand(0));
    if (rhsZero && rhsZero->getValue() == 0 && lhsCast
        && lhsCast->getCastOp() == "zext" && cmp->getPredicate() == "ne"
        && lhsCast->getOperand(0)->getTypeName() == "i1")
    {
        return lhsCast->getOperand(0);
    }

    auto* lhs = dynamic_cast<ConstantInt*>(inst->getOperand(0));
    auto* rhs = dynamic_cast<ConstantInt*>(inst->getOperand(1));
    if (!lhs || !rhs)
    {
        return nullptr;
    }

    int l = lhs->getValue();
    int r = rhs->getValue();
    const auto& pred = cmp->getPredicate();
    if (pred == "eq") return ConstantInt::getBool(l == r);
    if (pred == "ne") return ConstantInt::getBool(l != r);
    if (pred == "slt") return ConstantInt::getBool(l < r);
    if (pred == "sle") return ConstantInt::getBool(l <= r);
    if (pred == "sgt") return ConstantInt::getBool(l > r);
    if (pred == "sge") return ConstantInt::getBool(l >= r);
    return nullptr;
}

Value* ConstantFoldingPass::foldFCmp(Instruction* inst) const
{
    auto* cmp = dynamic_cast<FCmpInst*>(inst);
    auto* lhs = dynamic_cast<ConstantFloat*>(inst->getOperand(0));
    auto* rhs = dynamic_cast<ConstantFloat*>(inst->getOperand(1));
    if (!cmp || !lhs || !rhs)
    {
        return nullptr;
    }

    float l = lhs->getValue();
    float r = rhs->getValue();
    const auto& pred = cmp->getPredicate();
    if (pred == "oeq") return ConstantInt::getBool(l == r);
    if (pred == "one") return ConstantInt::getBool(l != r);
    if (pred == "olt") return ConstantInt::getBool(l < r);
    if (pred == "ole") return ConstantInt::getBool(l <= r);
    if (pred == "ogt") return ConstantInt::getBool(l > r);
    if (pred == "oge") return ConstantInt::getBool(l >= r);
    return nullptr;
}

Value* ConstantFoldingPass::foldCast(Instruction* inst) const
{
    auto* cast = dynamic_cast<CastInst*>(inst);
    if (!cast)
    {
        return nullptr;
    }

    Value* operand = inst->getOperand(0);
    if (cast->getCastOp() == "zext")
    {
        if (auto* intValue = dynamic_cast<ConstantInt*>(operand))
        {
            return ConstantInt::get(intValue->getValue() != 0 ? 1 : 0);
        }
    }
    if (cast->getCastOp() == "sitofp")
    {
        if (auto* intValue = dynamic_cast<ConstantInt*>(operand))
        {
            return ConstantFloat::get(static_cast<float>(intValue->getValue()));
        }
    }
    if (cast->getCastOp() == "fptosi")
    {
        if (auto* floatValue = dynamic_cast<ConstantFloat*>(operand))
        {
            return ConstantInt::get(static_cast<int>(std::trunc(floatValue->getValue())));
        }
    }
    return nullptr;
}
