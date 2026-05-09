#pragma once

#include "ir/core/Opcode.h"
#include "ir/core/User.h"

#include <string>
#include <utility>
#include <vector>

class BasicBlock;
class Function;

class Instruction : public User
{
protected:
    Opcode opcode;
    BasicBlock* parent;

    std::string resultPrefix() const;

public:
    Instruction(Type* ty, Opcode op, BasicBlock* bb = nullptr, const std::string& name = "");
    ~Instruction() override = default;

    Opcode getOpcode() const
    {
        return opcode;
    }

    BasicBlock* getParent() const
    {
        return parent;
    }

    void setParent(BasicBlock* bb)
    {
        parent = bb;
    }

    bool isTerminator() const
    {
        return opcode == Opcode::Ret || opcode == Opcode::Br;
    }

    std::string operandString() const override
    {
        return "%" + name;
    }
};

class AllocaInst : public Instruction
{
private:
    Type* allocatedType;

public:
    AllocaInst(Type* allocatedType, BasicBlock* bb = nullptr, const std::string& name = "");

    Type* getAllocatedType() const
    {
        return allocatedType;
    }

    std::string print() const override;
};

class LoadInst : public Instruction
{
private:
    Type* loadedType;

public:
    LoadInst(Type* loadedType, Value* ptr, BasicBlock* bb = nullptr, const std::string& name = "");

    Type* getLoadedType() const
    {
        return loadedType;
    }

    std::string print() const override;
};

class StoreInst : public Instruction
{
public:
    StoreInst(Value* value, Value* ptr, BasicBlock* bb = nullptr);

    std::string print() const override;
};

class BinaryOperator : public Instruction
{
public:
    BinaryOperator(Opcode op, Value* lhs, Value* rhs, BasicBlock* bb = nullptr, const std::string& name = "");

    std::string print() const override;
};

class ICmpInst : public Instruction
{
private:
    std::string predicate;

public:
    ICmpInst(const std::string& pred, Value* lhs, Value* rhs, BasicBlock* bb = nullptr, const std::string& name = "");

    const std::string& getPredicate() const
    {
        return predicate;
    }

    std::string print() const override;
};

class FCmpInst : public Instruction
{
private:
    std::string predicate;

public:
    FCmpInst(const std::string& pred, Value* lhs, Value* rhs, BasicBlock* bb = nullptr, const std::string& name = "");

    const std::string& getPredicate() const
    {
        return predicate;
    }

    std::string print() const override;
};

class BranchInst : public Instruction
{
private:
    BasicBlock* trueBlock;
    BasicBlock* falseBlock;

public:
    BranchInst(BasicBlock* target, BasicBlock* bb = nullptr);
    BranchInst(Value* condition, BasicBlock* trueBlock, BasicBlock* falseBlock, BasicBlock* bb = nullptr);

    bool isConditional() const
    {
        return getNumOperands() == 1;
    }

    BasicBlock* getTrueBlock() const
    {
        return trueBlock;
    }

    BasicBlock* getFalseBlock() const
    {
        return falseBlock;
    }

    std::string print() const override;
};

class ReturnInst : public Instruction
{
public:
    ReturnInst(Value* returnValue = nullptr, BasicBlock* bb = nullptr);

    std::string print() const override;
};

class CallInst : public Instruction
{
private:
    std::string calleeName;
    Type* returnType;

public:
    CallInst(Type* retTy, const std::string& calleeName, const std::vector<Value*>& args,
        BasicBlock* bb = nullptr, const std::string& name = "");

    const std::string& getCalleeName() const
    {
        return calleeName;
    }

    Type* getReturnType() const
    {
        return returnType;
    }

    std::string print() const override;
};

class GEPInst : public Instruction
{
private:
    Type* sourceElementType;
    Type* resultElementType;

public:
    GEPInst(Type* sourceElementType, Type* resultElementType, Value* basePtr,
        const std::vector<Value*>& indices, BasicBlock* bb = nullptr, const std::string& name = "");

    Type* getSourceElementType() const
    {
        return sourceElementType;
    }

    Type* getResultElementType() const
    {
        return resultElementType;
    }

    std::string print() const override;
};

class CastInst : public Instruction
{
private:
    std::string castOp;

public:
    CastInst(const std::string& castOp, Value* value, Type* destTy, BasicBlock* bb = nullptr, const std::string& name = "");

    const std::string& getCastOp() const
    {
        return castOp;
    }

    std::string print() const override;
};

class PhiInst : public Instruction
{
private:
    std::vector<std::pair<Value*, BasicBlock*>> incoming;

public:
    PhiInst(Type* ty, const std::vector<std::pair<Value*, BasicBlock*>>& incoming,
        BasicBlock* bb = nullptr, const std::string& name = "");

    std::string print() const override;
};

class SelectInst : public Instruction
{
public:
    SelectInst(Value* condition, Value* trueValue, Value* falseValue, BasicBlock* bb = nullptr, const std::string& name = "");

    std::string print() const override;
};

class VectorInst : public Instruction
{
public:
    VectorInst(Opcode op, Type* ty, const std::vector<Value*>& operands,
        BasicBlock* bb = nullptr, const std::string& name = "");

    std::string print() const override;
};
