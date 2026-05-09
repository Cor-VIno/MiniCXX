#pragma once

#include "ir/constants/Constant.h"

class ConstantCString : public Constant
{
private:
    std::string value;

public:
    explicit ConstantCString(const std::string& value);

    const std::string& getValue() const
    {
        return value;
    }

    std::string print() const override;
};
