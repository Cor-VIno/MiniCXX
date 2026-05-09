#include "ir/types/VectorType.h"

#include <map>
#include <utility>

static std::map<std::pair<Type*, uint64_t>, VectorType*> vectorTypeCache;

VectorType* VectorType::get(Type* elementType, uint64_t elementCount)
{
    auto key = std::make_pair(elementType, elementCount);
    auto it = vectorTypeCache.find(key);
    if (it != vectorTypeCache.end())
    {
        return it->second;
    }
    auto* ty = new VectorType(elementType, elementCount);
    vectorTypeCache[key] = ty;
    return ty;
}

std::string VectorType::toString() const
{
    return "<" + std::to_string(elementCount) + " x " + elementType->toString() + ">";
}
