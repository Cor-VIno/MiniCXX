#include "ir/constants/ConstantZeroInitializer.h"

#include <unordered_map>

static std::unordered_map<Type*, ConstantZeroInitializer*> zeroCache;

ConstantZeroInitializer* ConstantZeroInitializer::get(Type* ty)
{
    auto it = zeroCache.find(ty);
    if (it != zeroCache.end())
    {
        return it->second;
    }
    auto* value = new ConstantZeroInitializer(ty);
    zeroCache[ty] = value;
    return value;
}
