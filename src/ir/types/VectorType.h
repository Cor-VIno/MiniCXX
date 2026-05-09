#pragma once

#include "ir/types/Type.h"

#include <cstdint>

class VectorType : public Type
{
private:
    Type* elementType;
    uint64_t elementCount;

    VectorType(Type* elementType, uint64_t elementCount)
        : Type(VectorTyID), elementType(elementType), elementCount(elementCount)
    {
    }

public:
    Type* getElementType() const
    {
        return elementType;
    }

    uint64_t getElementCount() const
    {
        return elementCount;
    }

    std::string toString() const override;

    static VectorType* get(Type* elementType, uint64_t elementCount);
};
