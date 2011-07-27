#ifndef _MD5_H
#define _MD5_H

#include <time.h>

#include "stg/os_int.h"

#ifdef __cplusplus
extern "C" {
#endif

struct MD5Context {
	uint32_t buf[4];
	uint32_t bits[2];
	unsigned char in[64];
};

typedef struct MD5Context MD5_CTX;

char *crypt_make_salt(void);
void byteReverse(unsigned char*, unsigned);
void MD5Init(struct MD5Context *ctx);
void MD5Update(struct MD5Context*, char const*, unsigned);
void MD5Final(unsigned char digest[16], struct MD5Context *ctx);
void MD5Transform(uint32_t buf[4], uint32_t const in[16]);
char *libshadow_md5_crypt(const char*, const char*);
char *pw_encrypt(const char*, const char*);

/* AG functions */
char *make_ag_hash(time_t salt, const char *clear);
int check_ag_hash(time_t salt, const char *clear, const char *hash);

#ifdef __cplusplus
}
#endif

#endif
