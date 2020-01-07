#ifndef __ACTIONS_H__
#define __ACTIONS_H__

// Usage:
//
// ACTIONS_LIST actionsList;
// CLASS myClass;
// DATA1 myData1;
// DATA2 myData2;
//
// actionsList.Enqueue(myClass, &CLASS::myMethod1, myData1);
// actionsList.Enqueue(myClass, &CLASS::myMethod2, myData2);
//
// actionsList.InvokeAll();

#include <pthread.h>
#include <list>
#include <functional>

// Generalized actor type - a method of some class with one argument
template <class ACTIVE_CLASS, typename DATA_TYPE>
struct ACTOR
{
typedef void (ACTIVE_CLASS::*TYPE)(DATA_TYPE);
};

// Abstract base action class for polymorphic action invocation
class BASE_ACTION
{
public:
    virtual ~BASE_ACTION() {}
    virtual void Invoke() = 0;
};

// Concrete generalized action type - an actor with it's data and owner
template <class ACTIVE_CLASS, typename DATA_TYPE>
class ACTION : public BASE_ACTION,
               public std::unary_function<ACTIVE_CLASS &, void>
{
public:
    ACTION(ACTIVE_CLASS & ac,
           typename ACTOR<ACTIVE_CLASS, DATA_TYPE>::TYPE a,
           DATA_TYPE d)
        : activeClass(ac), actor(a), data(d) {}
    void Invoke();
private:
    ACTION(const ACTION<ACTIVE_CLASS, DATA_TYPE> & rvalue);
    ACTION<ACTIVE_CLASS, DATA_TYPE> & operator=(const ACTION<ACTIVE_CLASS, DATA_TYPE> & rvalue);

    ACTIVE_CLASS & activeClass;
    typename ACTOR<ACTIVE_CLASS, DATA_TYPE>::TYPE actor;
    DATA_TYPE data;
};

// A list of an actions
// All methods are thread-safe
class ACTIONS_LIST : private std::list<BASE_ACTION *>
{
public:
    // Just a typedef for parent class
    typedef std::list<BASE_ACTION *> parent;

    // Initialize mutex
    ACTIONS_LIST();
    // Delete actions and destroy mutex
    virtual ~ACTIONS_LIST();

    parent::iterator begin();
    parent::iterator end();
    parent::const_iterator begin() const;
    parent::const_iterator end() const;

    bool empty() const;
    size_t size() const;
    void swap(ACTIONS_LIST & list);

    // Add an action to list
    template <class ACTIVE_CLASS, typename DATA_TYPE>
    void Enqueue(ACTIVE_CLASS & ac,
                 typename ACTOR<ACTIVE_CLASS, DATA_TYPE>::TYPE a,
                 DATA_TYPE d);
    // Invoke all actions in the list
    void InvokeAll();
private:
    mutable pthread_mutex_t mutex;
};

#include "actions.inl.h"

#endif
