#pragma once

namespace STG
{

template <typename T>
class Optional
{
public:
    using value_type = T;

    Optional() noexcept : m_isSet(false) {}
    explicit Optional(const T& value) noexcept : m_value(value), m_isSet(true) {}

    Optional(const Optional<T>&) = default;
    Optional<T>& operator=(const Optional<T>&) = default;

    Optional(Optional<T>&&) = default;
    Optional<T>& operator=(Optional<T>&&) = default;

    Optional<T>& operator=(const T & rhs) noexcept
    {
        m_value = rhs;
        m_isSet = true;
        return *this;
    }

    const T & const_data() const noexcept { return m_value; }
    T & data() noexcept { return m_value; }
    const T & data() const noexcept { return m_value; }
    bool empty() const noexcept { return !m_isSet; }
    void reset() noexcept { m_isSet = false; }
    void splice(const Optional<T>& rhs) noexcept
    {
        if (rhs.m_isSet)
        {
            m_value = rhs.m_value;
            m_isSet = true;
        }
    }
    const T& get(const T& defaultValue) const noexcept
    {
        if (m_isSet)
            return m_value;
        else
            return defaultValue;
    }

private:
    value_type m_value;
    bool       m_isSet;
};

}
