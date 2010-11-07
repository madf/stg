 /*
 $Revision: 1.9 $
 $Date: 2010/03/11 14:42:04 $
 $Author: faust $
 */

/*
 * Copyright (c) 2001 by Peter Simons <simons@cryp.to>.
 * All rights reserved.
 */

#ifndef RESETABLE_VARIABLE_H
#define RESETABLE_VARIABLE_H

// This is a wrapper class about variables where you want to keep
// track of whether it has been assigened yet or not.

#include <iostream>

template <typename varT>
class RESETABLE
{
    template <typename varT1>
    friend std::ostream & operator<<(std::ostream & o, RESETABLE<varT1> v);
public:
    typedef varT value_type;

    //-------------------------------------------------------------------------
    RESETABLE()
        : value(),
          is_set(false)
    {
    }
    //-------------------------------------------------------------------------
    RESETABLE<value_type>(const RESETABLE<value_type> & rvalue)
        : value(rvalue.value),
          is_set(rvalue.is_set)
    {
    }
    //-------------------------------------------------------------------------
    RESETABLE(const value_type& val)
        : value(val),
          is_set(true)
    {
    }
    //-------------------------------------------------------------------------
    RESETABLE<value_type> & operator=(const RESETABLE<value_type> & rvalue)
    {
        value = rvalue.value;
        is_set = rvalue.is_set;
        return *this;
    }
    //-------------------------------------------------------------------------
    RESETABLE<value_type> & operator= (const value_type& rhs)
    {
        value = rhs;
        is_set = true;
        return *this;
    }
    //-------------------------------------------------------------------------
    const value_type& const_data() const throw()
    {
        return value;
    }
    //-------------------------------------------------------------------------
    value_type& data() throw()
    {
        return value;
    }
    //-------------------------------------------------------------------------
    operator const value_type&() const throw()
    {
        return value;
    }
    //-------------------------------------------------------------------------
    bool res_empty() const throw()
    {
        return !is_set;
    }
    //-------------------------------------------------------------------------
    void reset() throw()
    {
        is_set = false;
    }
    //-------------------------------------------------------------------------
protected:
    value_type  value;
    bool        is_set;
};
//-----------------------------------------------------------------------------
template <typename varT>
std::ostream & operator<<(std::ostream & o, RESETABLE<varT> v)
{
    return o << v.value;
}
//-------------------------------------------------------------------------
#endif // RESETABLE_VARIABLE_H

