#include "ir/core/GlobalVariable.h"

#include "ir/constants/Constant.h"
#include "ir/types/PointerType.h"

GlobalVariable::GlobalVariable(Type* valueType, const std::string& name, Constant* initializer, bool isConstant)
    : Value(PointerType::get(valueType), name), valueType(valueType), initializer(initializer), constant(isConstant)
{
}

std::string GlobalVariable::print() const
{
    std::string result = "@" + name + " = ";
    result += constant ? "constant " : "global ";
    result += valueType->toString();
    result += " ";
    result += initializer ? initializer->print() : "zeroinitializer";
    return result;
}
