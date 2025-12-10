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

void secure_decrypt_buffer(uint8_t *buf, uint32_t len)
{
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CBC_decrypt_buffer(&ctx, buf, len);
}
