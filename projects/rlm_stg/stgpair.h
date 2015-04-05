#ifndef __STG_STGPAIR_H__
#define __STG_STGPAIR_H__

#include <freeradius/libradius.h> // VALUE_PAIR

#define STGPAIR_KEYLENGTH 64
#define STGPAIR_VALUELENGTH 256

typedef struct STG_PAIR {
    char key[STGPAIR_KEYLENGTH];
    char value[STGPAIR_VALUELENGTH];
} STG_PAIR;

int emptyPair(const STG_PAIR* pair);

#endif
