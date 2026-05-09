#include "ir/types/FunctionType.h"

#include <map>
#include <utility>

static std::map<std::pair<Type*, std::vector<Type*>>, FunctionType*> functionTypeCache;

FunctionType* FunctionType::get(Type* returnType, const std::vector<Type*>& params)
{
    auto key = std::make_pair(returnType, params);
    auto it = functionTypeCache.find(key);
    if (it != functionTypeCache.end())
    {
        return it->second;
    }

    FunctionType* newType = new FunctionType(returnType, params);
    functionTypeCache[key] = newType;
    return newType;
}

std::string FunctionType::toString() const
{
    std::string result = returnType->toString() + " (";
    for (size_t i = 0; i < paramsTypes.size(); ++i)
    {
        result += paramsTypes[i]->toString();
        if (i + 1 < paramsTypes.size())
        {
            result += ", ";
        }
    }
    result += ")";
    return result;
}
