#pragma once

class Instruction;
class Module;

class DeadCodeEliminationPass
{
private:
    int removedInstructionCount = 0;

public:
    bool run(Module& module);

    int getRemovedInstructionCount() const
    {
        return removedInstructionCount;
    }

private:
    bool isRemovable(Instruction* inst) const;
};
