/*
 * crypto.c
 *
 *  Created on: Dec 10, 2025
 *      Author: agapitov
 */

#include "crypto.h"
#include "aes.h"
#include "key_template.h"

struct AES_ctx ctx;

void secure_init(const uint8_t iv[16])
{
    AES_init_ctx_iv(&ctx, aes_key, iv);
}

void secure_decrypt_buffer(uint8_t *buf, uint32_t len)
{
    AES_CTR_xcrypt_buffer(&ctx, buf, len);
}
