#pragma once

#include "ir/constants/Constant.h"

#include <vector>

class ConstantArray : public Constant
{
private:
    std::vector<Constant*> elements;

public:
    ConstantArray(Type* ty, std::vector<Constant*> elements);

    const std::vector<Constant*>& getElements() const
    {
        return elements;
    }

    std::string print() const override;
};
