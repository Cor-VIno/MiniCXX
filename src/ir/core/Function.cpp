#include "ir/core/Function.h"

#include "ir/core/BasicBlock.h"

#include <sstream>

Function::Function(FunctionType* funcTy, const std::string& name)
    : Value(funcTy, name), functionType(funcTy)
{
}

Function::Function(Type* retTy, const std::string& name)
    : Function(FunctionType::get(retTy, {}), name)
{
}

void Function::addBasicBlock(BasicBlock* bb)
{
    if (!bb)
    {
        return;
    }
    bb->setParent(this);
    bbList.push_back(bb);
}

Argument* Function::addArgument(Type* ty, const std::string& name)
{
    auto* arg = new Argument(ty, name, this, arguments.size());
    arguments.push_back(arg);
    return arg;
}

std::string Function::print() const
{
    std::ostringstream os;
    os << "define " << getReturnType()->toString() << " @" << name << "(";
    for (size_t i = 0; i < arguments.size(); ++i)
    {
        os << arguments[i]->typedOperandString();
        if (i + 1 < arguments.size())
        {
            os << ", ";
        }
    }
    os << ")";

    if (bbList.empty())
    {
        os << "\n";
        return os.str();
    }

    os << " {\n";
    for (const auto* bb : bbList)
    {
        if (bb)
        {
            os << bb->print();
        }
    }
    os << "}\n";
    return os.str();
}
