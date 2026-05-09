#pragma once

class Module;

class DeadStoreEliminationPass
{
private:
    int removedStoreCount = 0;

public:
    bool run(Module& module);

    int getRemovedStoreCount() const
    {
        return removedStoreCount;
    }
};
