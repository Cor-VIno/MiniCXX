#include "ir/constants/ConstantInt.h"
#include "ir/types/Type.h"
#include <unordered_map>

// 静态缓存池：映射关系为 [整数值 -> 对应的 ConstantInt 对象]
static std::unordered_map<int, ConstantInt*> intCache;

ConstantInt* ConstantInt::get(int val)
{
    // 查表：例如寻找数字 10
    auto it = intCache.find(val);
    if (it != intCache.end())
    {
        return it->second; // 找到了，直接返回历史对象
    }

    // 没找到，制造一个新的 i32 常量
    ConstantInt* newConst = new ConstantInt(Type::getInt32Ty(), val);

    // 放入缓存
    intCache[val] = newConst;

    return newConst;
}

ConstantInt* ConstantInt::getBool(bool val)
{
    return new ConstantInt(Type::getInt1Ty(), val ? 1 : 0);
}
