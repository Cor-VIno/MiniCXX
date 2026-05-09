#pragma once

#include "ir/core/Value.h"

#include <string>
#include <vector>

class Function;
class Instruction;

class BasicBlock : public Value
{
private:
    std::vector<Instruction*> instList;
    Function* parent;

public:
    BasicBlock(const std::string& name = "", Function* f = nullptr)
        : Value(Type::getLabelTy(), name), parent(f)
    {
    }

    ~BasicBlock() override = default;

    void addInstruction(Instruction* inst);
    void removeInstruction(Instruction* inst);
    Instruction* getTerminator() const;

    std::vector<Instruction*>& getInstructions()
    {
        return instList;
    }

    const std::vector<Instruction*>& getInstructions() const
    {
        return instList;
    }

    Function* getParent() const
    {
        return parent;
    }

    void setParent(Function* f)
    {
        parent = f;
    }

    std::string operandString() const override
    {
        return "%" + name;
    }

    std::string print() const override;
};
