#ifndef __PLUGIN_CREATOR_H__
#define __PLUGIN_CREATOR_H__

#include "noncopyable.h"

template <class T>
class PLUGIN_CREATOR : private NONCOPYABLE
{
public:
    PLUGIN_CREATOR() : plugin(new T()) {}
    //~PLUGIN_CREATOR() { delete plugin; }

    T * GetPlugin() { return plugin; }

private:
    T * plugin;
};

#endif
