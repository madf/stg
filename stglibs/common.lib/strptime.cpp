/*
 * $Revision: 1.1 $
 * $Date: 2007/05/17 08:25:58 $
 *
 * This file contain a replacement of commonly used function strptime
 * Under some OS's it appears only with _XOPEN_SOURCE definition
 *
 */

#define _XOPEN_SOURCE
#include <time.h>

#include "stg/common.h"

char * stg_strptime(const char * a, const char * b, struct tm * tm)
{
return strptime(a, b, tm);
}

