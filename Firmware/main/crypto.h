#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h> 
#include <stddef.h>
#include "mbedtls/ecdsa.h"

extern unsigned char signature_output[MBEDTLS_ECDSA_MAX_LEN]; 
extern size_t signature_len;

void sign_token(uint8_t token, uint8_t checksum);

#endif