#pragma once

#include "ir/types/Type.h"

#include <cstdint>

class ArrayType : public Type
{
private:
    Type* contained;
    uint64_t numElements;

    ArrayType(Type* elemType, uint64_t numElem)
        : Type(ArrayTyID), contained(elemType), numElements(numElem)
    {
    }

public:
    Type* getElementType() const
    {
        return contained;
    }

    uint64_t getNumElements() const
    {
        return numElements;
    }

    std::string toString() const override;

    static ArrayType* get(Type* elementType, uint64_t numElements);
};
