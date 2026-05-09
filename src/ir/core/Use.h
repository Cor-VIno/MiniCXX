#pragma once

class User;
class Value;

class Use
{
private:
    Value* value;
    User* user;

public:
    Use(Value* value = nullptr, User* user = nullptr)
        : value(value), user(user)
    {
    }

    Value* getValue() const
    {
        return value;
    }

    User* getUser() const
    {
        return user;
    }
};
