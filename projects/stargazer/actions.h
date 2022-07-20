#pragma once

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

#include <vector>
#include <functional>
#include <mutex>

// Generalized actor type - a method of some class with one argument
template <class ACTIVE_CLASS, typename DATA_TYPE>
struct ACTOR
{
    using TYPE = void (ACTIVE_CLASS::*)(DATA_TYPE);
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
class ACTION : public BASE_ACTION
{
public:
    ACTION(ACTIVE_CLASS & ac,
           typename ACTOR<ACTIVE_CLASS, DATA_TYPE>::TYPE a,
           DATA_TYPE d)
        : activeClass(ac), actor(a), data(d) {}
    void Invoke() override
    {
        (activeClass.*actor)(data);
    }
private:
    ACTION(const ACTION<ACTIVE_CLASS, DATA_TYPE> & rvalue);
    ACTION<ACTIVE_CLASS, DATA_TYPE> & operator=(const ACTION<ACTIVE_CLASS, DATA_TYPE> & rvalue);

    ACTIVE_CLASS & activeClass;
    typename ACTOR<ACTIVE_CLASS, DATA_TYPE>::TYPE actor;
    DATA_TYPE data;
};

// A list of an actions
// All methods are thread-safe
class ACTIONS_LIST
{
public:
    ~ACTIONS_LIST()
    {
        std::lock_guard lock(m_mutex);
        for (auto action : m_list)
            delete action;
    }

    auto begin() { std::lock_guard lock(m_mutex); return m_list.begin(); }
    auto end() { std::lock_guard lock(m_mutex); return m_list.end(); }
    auto begin() const { std::lock_guard lock(m_mutex); return m_list.begin(); }
    auto end() const { std::lock_guard lock(m_mutex); return m_list.end(); }

    bool empty() const { std::lock_guard lock(m_mutex); return m_list.empty(); }
    size_t size() const { std::lock_guard lock(m_mutex); return m_list.size(); }
    void swap(ACTIONS_LIST & rhs) { std::lock_guard lock(m_mutex); m_list.swap(rhs.m_list); }

    // Add an action to list
    template <class ACTIVE_CLASS, typename DATA_TYPE>
    void Enqueue(ACTIVE_CLASS & ac,
                 typename ACTOR<ACTIVE_CLASS, DATA_TYPE>::TYPE a,
                 DATA_TYPE d)
    {
        std::lock_guard lock(m_mutex);
        m_list.push_back(new ACTION<ACTIVE_CLASS, DATA_TYPE>(ac, a, d));
    }
    // Invoke all actions in the list
    void InvokeAll()
    {
        std::lock_guard lock(m_mutex);
        for (auto action : m_list)
            action->Invoke();
    }
private:
    mutable std::mutex m_mutex;
    std::vector<BASE_ACTION*> m_list;
};
