#pragma once

#include <unordered_set>

class BasicBlock;
class Module;

class SimplifyCFGPass
{
private:
    int removedBlockCount = 0;
    int simplifiedBranchCount = 0;

public:
    bool run(Module& module);

    int getRemovedBlockCount() const
    {
        return removedBlockCount;
    }

    int getSimplifiedBranchCount() const
    {
        return simplifiedBranchCount;
    }

private:
    bool simplifyConstantBranches(Module& module);
    bool removeUnreachableBlocks(Module& module);
    bool mergeSinglePredecessorBlocks(Module& module);
    void markReachable(BasicBlock* block, std::unordered_set<BasicBlock*>& reachable) const;
};
