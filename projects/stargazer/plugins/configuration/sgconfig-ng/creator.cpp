#include "stg/plugin_creator.h"
#include "stgconfig.h"

PLUGIN_CREATOR<STGCONFIG2> stgc;

BASE_PLUGIN * GetPlugin()
{
return stgc.GetPlugin();
}
