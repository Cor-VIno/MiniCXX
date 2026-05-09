#include "ir/core/Instruction.h"

#include "ir/core/BasicBlock.h"
#include "ir/types/PointerType.h"

#include <sstream>

static std::string opcodeName(Opcode op)
{
    switch (op)
    {
    case Opcode::Add: return "add";
    case Opcode::Sub: return "sub";
    case Opcode::Mul: return "mul";
    case Opcode::SDiv: return "sdiv";
    case Opcode::SRem: return "srem";
    case Opcode::FAdd: return "fadd";
    case Opcode::FSub: return "fsub";
    case Opcode::FMul: return "fmul";
    case Opcode::FDiv: return "fdiv";
    case Opcode::InsertElement: return "insertelement";
    case Opcode::ExtractElement: return "extractelement";
    default: return "";
    }
}

Instruction::Instruction(Type* ty, Opcode op, BasicBlock* bb, const std::string& name)
    : User(ty, name), opcode(op), parent(nullptr)
{
    if (bb)
    {
        bb->addInstruction(this);
    }
}

std::string Instruction::resultPrefix() const
{
    return name.empty() ? "" : "%" + name + " = ";
}

AllocaInst::AllocaInst(Type* allocatedType, BasicBlock* bb, const std::string& name)
    : Instruction(PointerType::get(allocatedType), Opcode::Alloca, bb, name), allocatedType(allocatedType)
{
}

std::string AllocaInst::print() const
{
    return resultPrefix() + "alloca " + allocatedType->toString();
}

LoadInst::LoadInst(Type* loadedType, Value* ptr, BasicBlock* bb, const std::string& name)
    : Instruction(loadedType, Opcode::Load, bb, name), loadedType(loadedType)
{
    addOperand(ptr);
}

std::string LoadInst::print() const
{
    return resultPrefix() + "load " + loadedType->toString() + ", " + getOperand(0)->typedOperandString();
}

StoreInst::StoreInst(Value* value, Value* ptr, BasicBlock* bb)
    : Instruction(Type::getVoidTy(), Opcode::Store, bb)
{
    addOperand(value);
    addOperand(ptr);
}

std::string StoreInst::print() const
{
    return "store " + getOperand(0)->typedOperandString() + ", " + getOperand(1)->typedOperandString();
}

BinaryOperator::BinaryOperator(Opcode op, Value* lhs, Value* rhs, BasicBlock* bb, const std::string& name)
    : Instruction(lhs ? lhs->getType() : Type::getInt32Ty(), op, bb, name)
{
    addOperand(lhs);
    addOperand(rhs);
}

std::string BinaryOperator::print() const
{
    return resultPrefix() + opcodeName(opcode) + " " + getTypeName() + " "
        + getOperand(0)->operandString() + ", " + getOperand(1)->operandString();
}

ICmpInst::ICmpInst(const std::string& pred, Value* lhs, Value* rhs, BasicBlock* bb, const std::string& name)
    : Instruction(Type::getInt1Ty(), Opcode::ICmp, bb, name), predicate(pred)
{
    addOperand(lhs);
    addOperand(rhs);
}

std::string ICmpInst::print() const
{
    return resultPrefix() + "icmp " + predicate + " " + getOperand(0)->getTypeName() + " "
        + getOperand(0)->operandString() + ", " + getOperand(1)->operandString();
}

FCmpInst::FCmpInst(const std::string& pred, Value* lhs, Value* rhs, BasicBlock* bb, const std::string& name)
    : Instruction(Type::getInt1Ty(), Opcode::FCmp, bb, name), predicate(pred)
{
    addOperand(lhs);
    addOperand(rhs);
}

std::string FCmpInst::print() const
{
    return resultPrefix() + "fcmp " + predicate + " " + getOperand(0)->getTypeName() + " "
        + getOperand(0)->operandString() + ", " + getOperand(1)->operandString();
}

BranchInst::BranchInst(BasicBlock* target, BasicBlock* bb)
    : Instruction(Type::getVoidTy(), Opcode::Br, bb), trueBlock(target), falseBlock(nullptr)
{
}

BranchInst::BranchInst(Value* condition, BasicBlock* trueBlock, BasicBlock* falseBlock, BasicBlock* bb)
    : Instruction(Type::getVoidTy(), Opcode::Br, bb), trueBlock(trueBlock), falseBlock(falseBlock)
{
    addOperand(condition);
}

std::string BranchInst::print() const
{
    if (isConditional())
    {
        return "br " + getOperand(0)->typedOperandString() + ", label %" + trueBlock->getName()
            + ", label %" + falseBlock->getName();
    }
    return "br label %" + trueBlock->getName();
}

ReturnInst::ReturnInst(Value* returnValue, BasicBlock* bb)
    : Instruction(Type::getVoidTy(), Opcode::Ret, bb)
{
    if (returnValue)
    {
        addOperand(returnValue);
    }
}

std::string ReturnInst::print() const
{
    if (getNumOperands() == 0)
    {
        return "ret void";
    }
    return "ret " + getOperand(0)->typedOperandString();
}

CallInst::CallInst(Type* retTy, const std::string& calleeName, const std::vector<Value*>& args,
    BasicBlock* bb, const std::string& name)
    : Instruction(retTy, Opcode::Call, bb, name), calleeName(calleeName), returnType(retTy)
{
    for (auto* arg : args)
    {
        addOperand(arg);
    }
}

std::string CallInst::print() const
{
    std::ostringstream os;
    os << (returnType->isVoidTy() ? "" : resultPrefix()) << "call " << returnType->toString()
        << " @" << calleeName << "(";
    for (size_t i = 0; i < getNumOperands(); ++i)
    {
        os << getOperand(i)->typedOperandString();
        if (i + 1 < getNumOperands())
        {
            os << ", ";
        }
    }
    os << ")";
    return os.str();
}

GEPInst::GEPInst(Type* sourceElementType, Type* resultElementType, Value* basePtr,
    const std::vector<Value*>& indices, BasicBlock* bb, const std::string& name)
    : Instruction(PointerType::get(resultElementType), Opcode::GEP, bb, name),
    sourceElementType(sourceElementType), resultElementType(resultElementType)
{
    addOperand(basePtr);
    for (auto* index : indices)
    {
        addOperand(index);
    }
}

std::string GEPInst::print() const
{
    std::ostringstream os;
    os << resultPrefix() << "getelementptr " << sourceElementType->toString() << ", "
        << getOperand(0)->typedOperandString();
    for (size_t i = 1; i < getNumOperands(); ++i)
    {
        os << ", " << getOperand(i)->typedOperandString();
    }
    return os.str();
}

CastInst::CastInst(const std::string& castOp, Value* value, Type* destTy, BasicBlock* bb, const std::string& name)
    : Instruction(destTy, Opcode::Cast, bb, name), castOp(castOp)
{
    addOperand(value);
}

std::string CastInst::print() const
{
    return resultPrefix() + castOp + " " + getOperand(0)->typedOperandString() + " to " + getTypeName();
}

PhiInst::PhiInst(Type* ty, const std::vector<std::pair<Value*, BasicBlock*>>& incoming,
    BasicBlock* bb, const std::string& name)
    : Instruction(ty, Opcode::Phi, bb, name), incoming(incoming)
{
    for (const auto& item : incoming)
    {
        addOperand(item.first);
    }
}

std::string PhiInst::print() const
{
    std::ostringstream os;
    os << resultPrefix() << "phi " << getTypeName() << " ";
    for (size_t i = 0; i < incoming.size(); ++i)
    {
        os << "[ " << incoming[i].first->operandString() << ", %" << incoming[i].second->getName() << " ]";
        if (i + 1 < incoming.size())
        {
            os << ", ";
        }
    }
    return os.str();
}

SelectInst::SelectInst(Value* condition, Value* trueValue, Value* falseValue, BasicBlock* bb, const std::string& name)
    : Instruction(trueValue ? trueValue->getType() : Type::getInt32Ty(), Opcode::Select, bb, name)
{
    addOperand(condition);
    addOperand(trueValue);
    addOperand(falseValue);
}

std::string SelectInst::print() const
{
    return resultPrefix() + "select " + getOperand(0)->typedOperandString() + ", "
        + getOperand(1)->typedOperandString() + ", " + getOperand(2)->typedOperandString();
}

VectorInst::VectorInst(Opcode op, Type* ty, const std::vector<Value*>& operands,
    BasicBlock* bb, const std::string& name)
    : Instruction(ty, op, bb, name)
{
    for (auto* operand : operands)
    {
        addOperand(operand);
    }
}

std::string VectorInst::print() const
{
    std::ostringstream os;
    os << resultPrefix() << opcodeName(opcode) << " " << getTypeName();
    for (auto* operand : getOperands())
    {
        os << " " << operand->typedOperandString();
    }
    return os.str();
}
