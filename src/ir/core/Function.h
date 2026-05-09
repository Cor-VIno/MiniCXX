#pragma once

#include "ir/core/Value.h"
#include "ir/types/FunctionType.h"

#include <string>
#include <vector>

class BasicBlock;
class Type;

class Function;

class Argument : public Value
{
private:
    Function* parent;
    size_t argNo;

public:
    Argument(Type* ty, const std::string& name = "", Function* f = nullptr, size_t no = 0)
        : Value(ty, name), parent(f), argNo(no)
    {
    }

    Function* getParent() const
    {
        return parent;
    }

    size_t getArgNo() const
    {
        return argNo;
    }

    std::string operandString() const override
    {
        return "%" + name;
    }

    std::string print() const override
    {
        return typedOperandString();
    }
};

class Function : public Value
{
private:
    std::vector<BasicBlock*> bbList;
    std::vector<Argument*> arguments;
    FunctionType* functionType;

public:
    Function(FunctionType* funcTy, const std::string& name);
    Function(Type* retTy, const std::string& name);

    ~Function() override = default;

    void addBasicBlock(BasicBlock* bb);
    Argument* addArgument(Type* ty, const std::string& name);

    std::vector<BasicBlock*>& getBasicBlocks()
    {
        return bbList;
    }

    const std::vector<BasicBlock*>& getBasicBlocks() const
    {
        return bbList;
    }

    const std::vector<Argument*>& getArguments() const
    {
        return arguments;
    }

    FunctionType* getFunctionType() const
    {
        return functionType;
    }

    Type* getReturnType() const
    {
        return functionType->getReturnType();
    }

    std::string operandString() const override
    {
        return "@" + name;
    }

    std::string print() const override;
};
