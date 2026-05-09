#include "ir/constants/ConstantCString.h"

#include "ir/types/ArrayType.h"
#include "ir/types/Type.h"

ConstantCString::ConstantCString(const std::string& value)
    : Constant(ArrayType::get(Type::getInt8Ty(), value.size() + 1)), value(value)
{
}

std::string ConstantCString::print() const
{
    std::string escaped;
    for (char ch : value)
    {
        if (ch == '\\' || ch == '"')
        {
            escaped += '\\';
        }
        escaped += ch;
    }
    return "c\"" + escaped + "\\00\"";
}
