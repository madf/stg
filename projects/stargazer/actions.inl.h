#ifndef __ACTIONS_INL_H__
#define __ACTIONS_INL_H__

#include <algorithm>

#include "stg/locker.h"

// Polymorphick action invocation
template <class ACTIVE_CLASS, typename DATA_TYPE>
inline
void ACTION<ACTIVE_CLASS, DATA_TYPE>::Invoke()
{
(activeClass.*actor)(data);
}

inline
ACTIONS_LIST::ACTIONS_LIST()
    : mutex()
{
pthread_mutex_init(&mutex, NULL);
}

// Delete all actions before deleting list
inline
ACTIONS_LIST::~ACTIONS_LIST()
{

    {
    STG_LOCKER lock(&mutex);

    parent::iterator it(parent::begin());
    while (it != parent::end())
        {
        delete *it++;
        }
    }

pthread_mutex_destroy(&mutex);
}

inline
ACTIONS_LIST::parent::iterator ACTIONS_LIST::begin()
{
STG_LOCKER lock(&mutex);
return parent::begin();
}

inline
ACTIONS_LIST::parent::iterator ACTIONS_LIST::end()
{
STG_LOCKER lock(&mutex);
return parent::end();
}

inline
ACTIONS_LIST::parent::const_iterator ACTIONS_LIST::begin() const
{
STG_LOCKER lock(&mutex);
return parent::begin();
}

inline
ACTIONS_LIST::parent::const_iterator ACTIONS_LIST::end() const
{
STG_LOCKER lock(&mutex);
return parent::end();
}

inline
bool ACTIONS_LIST::empty() const
{
STG_LOCKER lock(&mutex);
return parent::empty();
}

inline
size_t ACTIONS_LIST::size() const
{
STG_LOCKER lock(&mutex);
return parent::size();
}

inline
void ACTIONS_LIST::swap(ACTIONS_LIST & list)
{
STG_LOCKER lock(&mutex);
parent::swap(list);
}

template <class ACTIVE_CLASS, typename DATA_TYPE>
inline
void ACTIONS_LIST::Enqueue(ACTIVE_CLASS & ac,
                           typename ACTOR<ACTIVE_CLASS, DATA_TYPE>::TYPE a,
                           DATA_TYPE d)
{
STG_LOCKER lock(&mutex);
push_back(new ACTION<ACTIVE_CLASS, DATA_TYPE>(ac, a, d));
}

inline
void ACTIONS_LIST::InvokeAll()
{
STG_LOCKER lock(&mutex);
std::for_each(
        parent::begin(),
        parent::end(),
        [](auto action){ action->Invoke(); });
}

#endif
