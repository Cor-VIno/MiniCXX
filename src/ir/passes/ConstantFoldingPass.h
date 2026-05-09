#pragma once

class Instruction;
class Module;
class Value;

class ConstantFoldingPass
{
private:
    int foldedCount = 0;

public:
    bool run(Module& module);

    int getFoldedCount() const
    {
        return foldedCount;
    }

private:
    Value* foldInstruction(Instruction* inst) const;
    Value* foldBinary(Instruction* inst) const;
    Value* foldICmp(Instruction* inst) const;
    Value* foldFCmp(Instruction* inst) const;
    Value* foldCast(Instruction* inst) const;
};
