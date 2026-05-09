#pragma once

#include "ir/core/Value.h"

#include <vector>

class User : public Value
{
protected:
    std::vector<Value*> operands;

public:
    User(Type* t, const std::string& n = "") : Value(t, n)
    {
    }

    void addOperand(Value* v)
    {
        operands.push_back(v);
        if (v)
        {
            v->addUse(this);
        }
    }

    Value* getOperand(size_t i) const
    {
        return operands[i];
    }

    void setOperand(size_t i, Value* v)
    {
        if (operands[i])
        {
            operands[i]->removeUse(this);
        }
        operands[i] = v;
        if (v)
        {
            v->addUse(this);
        }
    }

    void replaceOperand(Value* oldValue, Value* newValue)
    {
        for (auto& operand : operands)
        {
            if (operand == oldValue)
            {
                if (operand)
                {
                    operand->removeUse(this);
                }
                operand = newValue;
                if (newValue)
                {
                    newValue->addUse(this);
                }
            }
        }
    }

    void dropAllOperands()
    {
        for (auto* operand : operands)
        {
            if (operand)
            {
                operand->removeUse(this);
            }
        }
        operands.clear();
    }

    size_t getNumOperands() const
    {
        return operands.size();
    }

    const std::vector<Value*>& getOperands() const
    {
        return operands;
    }
};
