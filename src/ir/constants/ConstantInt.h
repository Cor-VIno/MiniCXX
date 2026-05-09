#pragma once
#include "Constant.h"

class ConstantInt : public Constant
{
private:
    int value;

    // 私有构造，强制用户使用 get() 获取实例
    ConstantInt(Type* ty, int val) : Constant(ty), value(val)
    {
    }

public:
    int getValue() const
    {
        return value;
    }

    std::string print() const override
    {
        return std::to_string(value);
    }

    // 核心：静态工厂方法
    static ConstantInt* get(int val);
    static ConstantInt* getBool(bool val);
};
