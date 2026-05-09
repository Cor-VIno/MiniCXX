#pragma once

#include <string>
#include <vector>

class Module;

class IRVerifier
{
private:
    std::vector<std::string> errors;

public:
    bool verify(const Module& module);
    void printErrors() const;
};
