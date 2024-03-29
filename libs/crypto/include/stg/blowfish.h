/*
 * Author     :  Paul Kocher
 * E-mail     :  pck@netcom.com
 * Date       :  1997
 * Description:  C implementation of the Blowfish algorithm.
 */

#ifndef BLOWFISH_H
#define BLOWFISH_H

#define MAXKEYBYTES 56          /* 448 bits */

#ifdef __cplusplus
#include <cstddef> // size_t
#include <cstdint>
extern "C" {
#else
#include <stddef.h> // size_t
#include <stdint.h>
#endif

typedef struct {
  uint32_t P[16 + 2];
  uint32_t S[4][256];
} BLOWFISH_CTX;

void Blowfish_Init(BLOWFISH_CTX * ctx, void* key, int keyLen);
void Blowfish_Encrypt(const BLOWFISH_CTX * ctx, uint32_t * xl, uint32_t * xr);
void Blowfish_Decrypt(const BLOWFISH_CTX * ctx, uint32_t * xl, uint32_t * xr);

void InitContext(const char * key, size_t length, BLOWFISH_CTX * ctx);
void DecryptBlock(void * d, const void * s, const BLOWFISH_CTX * ctx);
void EncryptBlock(void * d, const void * s, const BLOWFISH_CTX * ctx);

void DecryptString(void * d, const void * s, size_t length, const BLOWFISH_CTX * ctx);
void EncryptString(void * d, const void * s, size_t length, const BLOWFISH_CTX * ctx);

#ifdef __cplusplus
}
#endif

#endif

