#ifndef __USER_TC_IFACE_H__
#define __USER_TC_IFACE_H__

#ifdef HAVE_STDINT
    #include <stdint.h>
#else
    #ifdef HAVE_INTTYPES
        #include <inttypes.h>
    #else
        #error "You need either stdint.h or inttypes.h to compile this!"
    #endif
#endif

#include "tc_packets.h"

namespace STG
{

    class IUSER_TC
    {
    public:
        virtual ~IUSER_TC() {};
        virtual void AddIP(uint32_t) = 0;
        virtual void DeleteIP(uint32_t, TRAFF_DATA *) = 0;
        virtual void GetIP(uint32_t, TRAFF_DATA *) = 0;
    };

}

#endif
