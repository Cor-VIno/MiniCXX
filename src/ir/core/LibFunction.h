#pragma once

#include "ir/core/Function.h"

class LibFunction : public Function
{
private:
    bool variadic = false;

public:
    LibFunction(FunctionType* funcTy, const std::string& name, bool variadic = false)
        : Function(funcTy, name), variadic(variadic)
    {
    }

    std::string print() const override;
};
