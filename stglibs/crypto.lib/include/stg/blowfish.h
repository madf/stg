/*
 * Author     :  Paul Kocher
 * E-mail     :  pck@netcom.com
 * Date       :  1997
 * Description:  C implementation of the Blowfish algorithm.
 */

#ifndef BLOWFISH_H
#define BLOWFISH_H

#include "stg/os_int.h"

#define MAXKEYBYTES 56          /* 448 bits */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t P[16 + 2];
  uint32_t S[4][256];
} BLOWFISH_CTX;

void Blowfish_Init(BLOWFISH_CTX *ctx, unsigned char *key, int keyLen);
void Blowfish_Encrypt(BLOWFISH_CTX *ctx, uint32_t *xl, uint32_t *xr);
void Blowfish_Decrypt(BLOWFISH_CTX *ctx, uint32_t *xl, uint32_t *xr);

void EnDecodeInit(const char * key, size_t length, BLOWFISH_CTX *ctx);
void DecodeString(char * d, const char * s, BLOWFISH_CTX *ctx);
void EncodeString(char * d, const char * s, BLOWFISH_CTX *ctx);

#ifdef __cplusplus
}
#endif

#endif

