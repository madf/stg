/*
 * Copyright (c) 2001 by Peter Simons <simons@cryp.to>.
 * All rights reserved.
 */

#ifndef RESETABLE_VARIABLE_H
#define RESETABLE_VARIABLE_H

// This is a wrapper class about variables where you want to keep
// track of whether it has been assigened yet or not.

template <typename T>
class RESETABLE
{
public:
    typedef T value_type;

    RESETABLE() : value(), is_set(false) {}
    RESETABLE(const T & v) : value(v), is_set(true) {}

    RESETABLE(const RESETABLE<T> & rvalue)
        : value(rvalue.value),
          is_set(rvalue.is_set)
    {}

    RESETABLE<T> & operator=(const RESETABLE<T> & rhs)
    {
        value = rhs.value;
        is_set = rhs.is_set;
        return *this;
    }

    RESETABLE<T> & operator=(const T & rhs)
    {
        value = rhs;
        is_set = true;
        return *this;
    }

    const T & const_data() const throw() { return value; }
    T & data() throw() { return value; }
    const T & data() const throw() { return value; }
    bool empty() const throw() { return !is_set; }
    void reset() throw() { is_set = false; }
    void splice(const RESETABLE<T> & rhs)
    {
        if (rhs.is_set)
        {
            value = rhs.value;
            is_set = true;
        }
    }
    void maybeSet(value_type& dest) const
    {
        if (is_set)
            dest = value;
    }

private:
    value_type value;
    bool       is_set;
};

#endif // RESETABLE_VARIABLE_H
