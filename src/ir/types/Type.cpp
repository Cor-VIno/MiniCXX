#include "ir/types/Type.h"

class VoidType : public Type
{
public:
    VoidType() : Type(VoidTyID)
    {
    }

    std::string toString() const override
    {
        return "void";
    }
};

class LabelType : public Type
{
public:
    LabelType() : Type(LabelTyID)
    {
    }

    std::string toString() const override
    {
        return "label";
    }
};

class FloatType : public Type
{
public:
    FloatType() : Type(FloatTyID)
    {
    }

    std::string toString() const override
    {
        return "float";
    }
};

class IntegerType : public Type
{
private:
    int bitWidth;

public:
    explicit IntegerType(int width) : Type(IntegerTyID), bitWidth(width)
    {
    }

    std::string toString() const override
    {
        return "i" + std::to_string(bitWidth);
    }
};

static VoidType GlobalVoidTy;
static LabelType GlobalLabelTy;
static FloatType GlobalFloatTy;
static IntegerType GlobalInt8Ty(8);
static IntegerType GlobalInt32Ty(32);
static IntegerType GlobalInt1Ty(1);

Type* Type::getVoidTy()
{
    return &GlobalVoidTy;
}

Type* Type::getLabelTy()
{
    return &GlobalLabelTy;
}

Type* Type::getFloatTy()
{
    return &GlobalFloatTy;
}

Type* Type::getInt8Ty()
{
    return &GlobalInt8Ty;
}

Type* Type::getInt32Ty()
{
    return &GlobalInt32Ty;
}

Type* Type::getInt1Ty()
{
    return &GlobalInt1Ty;
}
