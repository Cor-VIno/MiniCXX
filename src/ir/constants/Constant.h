#pragma once

#include "ir/core/User.h"

class Constant : public User
{
protected:
    explicit Constant(Type* ty) : User(ty, "")
    {
    }

public:
    ~Constant() override = default;
};
