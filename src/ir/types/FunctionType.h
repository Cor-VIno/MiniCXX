#pragma once

#include "ir/types/Type.h"

#include <vector>

class FunctionType : public Type
{
private:
    Type* returnType;
    std::vector<Type*> paramsTypes;

    FunctionType(Type* retTy, const std::vector<Type*>& params)
        : Type(FunctionTyID), returnType(retTy), paramsTypes(params)
    {
    }

public:
    Type* getReturnType() const
    {
        return returnType;
    }

    const std::vector<Type*>& getParamsTypes() const
    {
        return paramsTypes;
    }

    std::string toString() const override;

    static FunctionType* get(Type* returnType, const std::vector<Type*>& params);
};
