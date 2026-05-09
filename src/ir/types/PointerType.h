#pragma once

#include "ir/types/Type.h"

class PointerType : public Type
{
private:
    Type* contained;

    explicit PointerType(Type* contained) : Type(PointerTyID), contained(contained)
    {
    }

public:
    Type* getElementType() const
    {
        return contained;
    }

    std::string toString() const override
    {
        return "ptr";
    }

    static PointerType* get(Type* elementType);
};
