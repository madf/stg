#ifndef STG_COMP_STAT_H
#define STG_COMP_STAT_H

#ifdef LINUX
#include <stdint.h>
#endif

#ifdef FREE_BSD5
#include <inttypes.h>
#endif

#ifdef FREE_BSD
#include <sys/inttypes.h>
#endif

#include "stg_const.h"
//-----------------------------------------------------------------------------
struct DAY_STAT
{
    DAY_STAT()
    {
        lastUpdate = 0;
        memset(upload,      0, sizeof(uint64_t) * DIR_NUM);
        memset(download,    0, sizeof(uint64_t) * DIR_NUM);
        memset(cash,        0, sizeof(double) * DIR_NUM);
    }

    time_t      lastUpdate;
    uint64_t    upload[DIR_NUM];
    uint64_t    download[DIR_NUM];
    double      cash[DIR_NUM];
};
//-----------------------------------------------------------------------------
struct MONTH_STAT
{
    DAY_STAT    dayStat[31];
    char        notUsed;
};
//-----------------------------------------------------------------------------

#endif //STG_COMP_STAT_H
