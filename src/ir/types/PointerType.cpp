#include "ir/types/PointerType.h"
#include <unordered_map>

// 静态缓存池：映射关系为 [指向的类型 -> 对应的指针类型]
static std::unordered_map<Type*, PointerType*> pointerTypeCache;

PointerType* PointerType::get(Type* elementType)
{
    // 1. 查表：如果这个类型的指针已经存在，直接返回
    auto it = pointerTypeCache.find(elementType);
    if (it != pointerTypeCache.end())
    {
        return it->second;
    }

    // 2. 如果不存在，创建新的 PointerType
    PointerType* newType = new PointerType(elementType);

    // 3. 存入缓存池
    pointerTypeCache[elementType] = newType;

    return newType;
}