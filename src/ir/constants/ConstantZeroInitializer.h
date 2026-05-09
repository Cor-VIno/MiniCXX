#pragma once

#include "ir/constants/Constant.h"

class ConstantZeroInitializer : public Constant
{
private:
    explicit ConstantZeroInitializer(Type* ty) : Constant(ty)
    {
    }

public:
    static ConstantZeroInitializer* get(Type* ty);

    std::string print() const override
    {
        return "zeroinitializer";
    }
};
