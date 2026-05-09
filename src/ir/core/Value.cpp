#include "ir/core/Value.h"

#include "ir/core/User.h"

void Value::removeUse(User* u)
{
    for (auto it = useList.begin(); it != useList.end();)
    {
        if (it->getUser() == u)
        {
            it = useList.erase(it);
            continue;
        }
        ++it;
    }
}

void Value::replaceAllUsesWith(Value* newValue)
{
    if (!newValue || newValue == this)
    {
        return;
    }

    auto oldUses = useList;
    for (const auto& use : oldUses)
    {
        if (User* user = use.getUser())
        {
            user->replaceOperand(this, newValue);
        }
    }
    useList.clear();
}
