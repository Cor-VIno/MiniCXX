#pragma once

class Module;

class StoreLoadForwardingPass
{
private:
    int forwardedLoadCount = 0;

public:
    bool run(Module& module);

    int getForwardedLoadCount() const
    {
        return forwardedLoadCount;
    }
};
