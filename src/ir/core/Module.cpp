#include "ir/core/Module.h"

#include "ir/core/Function.h"
#include "ir/core/GlobalVariable.h"

#include <sstream>

void Module::addGlobal(GlobalVariable* global)
{
    if (global)
    {
        globals.push_back(global);
    }
}

void Module::addFunction(Function* function)
{
    if (function)
    {
        functions.push_back(function);
    }
}

Function* Module::getFunction(const std::string& name) const
{
    for (auto* function : functions)
    {
        if (function && function->getName() == name)
        {
            return function;
        }
    }
    return nullptr;
}

std::string Module::print() const
{
    std::ostringstream os;
    for (const auto* global : globals)
    {
        os << global->print() << "\n";
    }
    if (!globals.empty() && !functions.empty())
    {
        os << "\n";
    }
    for (const auto* function : functions)
    {
        os << function->print() << "\n";
    }
    return os.str();
}
