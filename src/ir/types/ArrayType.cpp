#include "ir/types/ArrayType.h"

#include <map>
#include <utility>

static std::map<std::pair<Type*, uint64_t>, ArrayType*> arrayTypeCache;

ArrayType* ArrayType::get(Type* elementType, uint64_t numElements)
{
    auto key = std::make_pair(elementType, numElements);
    auto it = arrayTypeCache.find(key);
    if (it != arrayTypeCache.end())
    {
        return it->second;
    }

    ArrayType* newType = new ArrayType(elementType, numElements);
    arrayTypeCache[key] = newType;
    return newType;
}

std::string ArrayType::toString() const
{
    return "[" + std::to_string(numElements) + " x " + contained->toString() + "]";
}
