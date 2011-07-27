#ifndef __PLUGIN_CREATOR_H__
#define __PLUGIN_CREATOR_H__

template <class T>
class PLUGIN_CREATOR
{
public:
    PLUGIN_CREATOR() : plugin(new T()) {}
    ~PLUGIN_CREATOR() { delete plugin; }

    T * GetPlugin() { return plugin; }

private:
    T * plugin;
};

#endif
