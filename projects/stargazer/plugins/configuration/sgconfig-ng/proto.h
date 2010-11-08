#ifndef __PROTO_H__
#define __PROTO_H__

#define PROTO_MAGIC "12345678"

namespace REQ {
    struct HEADER {
        char magic[8];
        uint32_t version;
        char login[36];
    };

    struct CRYPTO_HEADER {
        char login[36];
        uint32_t dataSize;
    };
}

namespace RESP {
    enum {
        OK = 0,
        INVALID_MAGIC,
        UNSUPPORTED_VERSION,
        INVALID_CREDENTIALS
    };

    struct HEADER {
        char magic[8];
        uint32_t version;
        uint32_t code;
    };

    struct CRYPTO_HEADER {
        char login[36];
        uint32_t dataSize;
    };
}

#endif
