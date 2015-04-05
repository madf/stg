#include "stgpair.h"

int emptyPair(const STG_PAIR* pair)
{
    return pair != NULL && pair->key[0] != '\0' && pair->value[0] != '\0';
}
