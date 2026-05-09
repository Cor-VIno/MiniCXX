#pragma once

#include "ir/core/Use.h"
#include "ir/types/Type.h"

#include <string>
#include <vector>

class User;

class Value
{
protected:
    Type* type;
    std::string name;
    std::vector<Use> useList;

public:
    Value(Type* t, const std::string& n = "") : type(t), name(n)
    {
    }

    virtual ~Value() = default;

    Type* getType() const
    {
        return type;
    }

    const std::string& getName() const
    {
        return name;
    }

    void setName(const std::string& n)
    {
        name = n;
    }

    void addUse(User* u)
    {
        useList.emplace_back(this, u);
    }

    void removeUse(User* u);

    bool useEmpty() const
    {
        return useList.empty();
    }

    void replaceAllUsesWith(Value* newValue);

    std::string getTypeName() const
    {
        return type ? type->toString() : "void";
    }

    virtual std::string operandString() const
    {
        return name.empty() ? print() : name;
    }

    virtual std::string typedOperandString() const
    {
        return getTypeName() + " " + operandString();
    }

    virtual std::string print() const = 0;
};
