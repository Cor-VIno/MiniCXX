#include "ir/core/UndefValue.h"

#include <unordered_map>

static std::unordered_map<Type*, UndefValue*> undefCache;

UndefValue* UndefValue::get(Type* ty)
{
    auto it = undefCache.find(ty);
    if (it != undefCache.end())
    {
        return it->second;
    }
    auto* value = new UndefValue(ty);
    undefCache[ty] = value;
    return value;
}
