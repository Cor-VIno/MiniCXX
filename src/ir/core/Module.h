#pragma once

#include <string>
#include <vector>

class Function;
class GlobalVariable;

class Module
{
private:
    std::vector<GlobalVariable*> globals;
    std::vector<Function*> functions;

public:
    void addGlobal(GlobalVariable* global);
    void addFunction(Function* function);
    Function* getFunction(const std::string& name) const;

    const std::vector<GlobalVariable*>& getGlobals() const
    {
        return globals;
    }

    const std::vector<Function*>& getFunctions() const
    {
        return functions;
    }

    std::string print() const;
};
