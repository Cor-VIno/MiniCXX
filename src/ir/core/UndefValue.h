#pragma once

#include "ir/core/Value.h"

class UndefValue : public Value
{
private:
    explicit UndefValue(Type* ty) : Value(ty)
    {
    }

public:
    static UndefValue* get(Type* ty);

    std::string print() const override
    {
        return "undef";
    }
};
