#ifndef __NONCOPYABLE_H__
#define __NONCOPYABLE_H__

class NONCOPYABLE
{
protected:
    NONCOPYABLE() {}
    virtual ~NONCOPYABLE() {}
private:  // emphasize the following members are private
    NONCOPYABLE(const NONCOPYABLE &);
    const NONCOPYABLE & operator=(const NONCOPYABLE &);
};

#endif
