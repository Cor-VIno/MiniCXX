#include "ir/core/LibFunction.h"

#include <sstream>

std::string LibFunction::print() const
{
    std::ostringstream os;
    os << "declare " << getReturnType()->toString() << " @" << getName() << "(";
    const auto& params = getFunctionType()->getParamsTypes();
    for (size_t i = 0; i < params.size(); ++i)
    {
        os << params[i]->toString();
        if (i + 1 < params.size() || variadic)
        {
            os << ", ";
        }
    }
    if (variadic)
    {
        os << "...";
    }
    os << ")\n";
    return os.str();
}
