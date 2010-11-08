#include "stgconfig.h"

class STGCONFIG_CREATOR
{
private:
    STGCONFIG2 * stgconfig;

public:
    STGCONFIG_CREATOR()
        : stgconfig(new STGCONFIG2())
        {
        };
    ~STGCONFIG_CREATOR()
        {
        delete stgconfig;
        };

    STGCONFIG2 * GetPlugin()
        {
        return stgconfig;
        };
};

STGCONFIG_CREATOR stgc;

BASE_PLUGIN * GetPlugin()
{
return stgc.GetPlugin();
}
