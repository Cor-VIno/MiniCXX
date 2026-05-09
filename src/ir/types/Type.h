#pragma once

#include <iostream>
#include <string>

class Type
{
public:
    enum TypeID
    {
        VoidTyID,
        LabelTyID,
        IntegerTyID,
        FloatTyID,
        PointerTyID,
        ArrayTyID,
        FunctionTyID,
        VectorTyID
    };

protected:
    TypeID tid;

    explicit Type(TypeID id) : tid(id)
    {
    }

public:
    virtual ~Type() = default;

    TypeID getTypeID() const
    {
        return tid;
    }

    bool isVoidTy() const
    {
        return tid == VoidTyID;
    }
    bool isLabelTy() const
    {
        return tid == LabelTyID;
    }
    bool isIntegerTy() const
    {
        return tid == IntegerTyID;
    }
    bool isFloatTy() const
    {
        return tid == FloatTyID;
    }
    bool isPointerTy() const
    {
        return tid == PointerTyID;
    }
    bool isArrayTy() const
    {
        return tid == ArrayTyID;
    }
    bool isFunctionTy() const
    {
        return tid == FunctionTyID;
    }
    bool isVectorTy() const
    {
        return tid == VectorTyID;
    }

    static Type* getVoidTy();
    static Type* getLabelTy();
    static Type* getFloatTy();
    static Type* getInt8Ty();
    static Type* getInt32Ty();
    static Type* getInt1Ty();

    virtual std::string toString() const = 0;

    virtual void print() const
    {
        std::cout << toString();
    }
};
