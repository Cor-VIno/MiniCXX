#pragma once

#include "ir/core/Value.h"

class Constant;

class GlobalVariable : public Value
{
private:
    Type* valueType;
    Constant* initializer;
    bool constant;

public:
    GlobalVariable(Type* valueType, const std::string& name, Constant* initializer = nullptr, bool isConstant = false);

    Type* getValueType() const
    {
        return valueType;
    }

    Constant* getInitializer() const
    {
        return initializer;
    }

    bool isConstant() const
    {
        return constant;
    }

    std::string operandString() const override
    {
        return "@" + name;
    }

    std::string print() const override;
};
