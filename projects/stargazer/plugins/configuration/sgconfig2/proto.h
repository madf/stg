#ifndef __PROTO_H__
#define __PROTO_H__

namespace SGCONF2 {

/*
 * --- Protocol structure (binary part) ---
 *
 *  Request:
 *  |---------------|
 *  |PROTOHEADER    |
 *  |REQUESTHEADER  |
 *  |---------------|
 *  |   cryptodata  |
 *  ~~~~~~~~~~~~~~~~~
 *  |---------------|
 *
 *  Response:
 *  |---------------|
 *  |PROTOHEADER    |
 *  |RESPONSEHEADER |
 *  | error message |
 *  |   cryptodata  |
 *  ~~~~~~~~~~~~~~~~~
 *  |---------------|
 *
 */

    static char magic[8] = "STGCONF2";

    enum RESPONSECODES {
	E_OK = 0,	// No error
	E_NET_ERROR,	// Network error (i.e. - timeout)
	E_PROTO_ERROR,  // Protocol error (invalid magic, unsupported version, etc.)
	E_INVALID_LOGIN,// Invalid login
	E_PERMISSIONS   // Operation not permitted
    };

    struct PROTOHEADER {
	char     magic[8];
	uint32_t version;
    };

    struct REQUESTHEADER {
	char     login[32];
    };

    struct CRYPTOHEADER {
	char     login[32];
	uint32_t dataSize; // Can't be 0
    };

    struct RESPONSEHEADER {
	uint32_t code;
	uint32_t errorMessageSize; // May be 0
	uint32_t dataSize; // May be 0
    };

}

#endif
