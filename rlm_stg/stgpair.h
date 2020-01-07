#ifndef __STG_STGPAIR_H__
#define __STG_STGPAIR_H__

#include <stddef.h>

#define STGPAIR_KEYLENGTH 64
#define STGPAIR_VALUELENGTH 256

#ifdef __cplusplus
extern "C" {
#endif

typedef struct STG_PAIR {
    char key[STGPAIR_KEYLENGTH];
    char value[STGPAIR_VALUELENGTH];
} STG_PAIR;

typedef struct STG_RESULT {
    STG_PAIR* modify;
    STG_PAIR* reply;
    int returnCode;
} STG_RESULT;

inline
int emptyPair(const STG_PAIR* pair)
{
    return pair == NULL || pair->key[0] == '\0' || pair->value[0] == '\0';
}

enum
{
    STG_REJECT,
    STG_FAIL,
    STG_OK,
    STG_HANDLED,
    STG_INVALID,
    STG_USERLOCK,
    STG_NOTFOUND,
    STG_NOOP,
    STG_UPDATED
};

#ifdef __cplusplus
}
#endif

#endif
