#include "ir/constants/ConstantArray.h"

#include <sstream>

ConstantArray::ConstantArray(Type* ty, std::vector<Constant*> elements)
    : Constant(ty), elements(std::move(elements))
{
}

std::string ConstantArray::print() const
{
    std::ostringstream os;
    os << "[";
    for (size_t i = 0; i < elements.size(); ++i)
    {
        os << (elements[i] ? elements[i]->typedOperandString() : "void zeroinitializer");
        if (i + 1 < elements.size())
        {
            os << ", ";
        }
    }
    os << "]";
    return os.str();
}
