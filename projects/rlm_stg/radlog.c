#include "radlog.h"

//#ifndef NDEBUG
//#define NDEBUG
#include <freeradius/ident.h>
#include <freeradius/radiusd.h>
#include <freeradius/modules.h>
//#undef NDEBUG
//#endif

#include <stdarg.h>

void RadLog(const char* format, ...)
{
    char buf[1024];

    va_list vl;
    va_start(vl, format);
    vsnprintf(buf, sizeof(buf), format, vl);
    va_end(vl);

    DEBUG("[rlm_stg] *** %s", buf);
}
