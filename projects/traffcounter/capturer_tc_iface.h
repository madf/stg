#ifndef __CAPTURER_TC_IFACE_H__
#define __CAPTURER_TC_IFACE_H__

#ifdef HAVE_STDINT
    #include <stdint.h>
#else
    #ifdef HAVE_INTTYPES
        #include <inttypes.h>
    #else
        #error "You need either stdint.h or inttypes.h to compile this!"
    #endif
#endif

namespace STG
{

    class ICAPTURER_TC
    {
    public:
        virtual ~ICAPTURER_TC() {};
        virtual void AddPacket(const iphdr &, uint16_t, uint16_t) = 0;
    };

}

#endif
