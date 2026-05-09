#include "ir/constants/ConstantFloat.h"

#include "ir/types/Type.h"

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <unordered_map>

static std::unordered_map<uint32_t, ConstantFloat*> floatCache;

ConstantFloat* ConstantFloat::get(float val)
{
    uint32_t bitBits = 0;
    std::memcpy(&bitBits, &val, sizeof(float));

    auto it = floatCache.find(bitBits);
    if (it != floatCache.end())
    {
        return it->second;
    }

    ConstantFloat* newConst = new ConstantFloat(Type::getFloatTy(), val);
    floatCache[bitBits] = newConst;
    return newConst;
}

std::string ConstantFloat::print() const
{
    std::ostringstream os;
    os << std::showpoint << value;
    return os.str();
}
