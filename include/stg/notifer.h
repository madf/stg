#pragma once

namespace STG
{

template <typename... Ts>
struct NotifierBase
{
    virtual ~NotifierBase() = default;

    virtual void notify(const Ts&... values) = 0;
};

template <typename T>
using PropertyNotifierBase = NotifierBase<T, T>;

}
