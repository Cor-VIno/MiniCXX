#pragma once

#include "ir/constants/Constant.h"

class ConstantFloat : public Constant
{
private:
    float value;

    ConstantFloat(Type* ty, float val) : Constant(ty), value(val)
    {
    }

public:
    float getValue() const
    {
        return value;
    }

    std::string print() const override;

    static ConstantFloat* get(float val);
};
