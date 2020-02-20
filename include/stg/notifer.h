#pragma once

namespace STG
{

template <typename T>
struct PropertyNotifierBase
{
    virtual ~PropertyNotifierBase() = default;

    virtual void Notify(const T& oldValue, const T& newValue) = 0;
};

template <typename T>
struct NotifierBase
{
    virtual ~NotifierBase() = default;

    virtual void Notify(const T& value) = 0;
};

}
