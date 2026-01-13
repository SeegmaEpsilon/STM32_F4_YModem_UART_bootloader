/*
 * crypto.h
 *
 *  Created on: Dec 10, 2025
 *      Author: agapitov
 */

#ifndef USER_CRYPTO_CRYPTO_H_
#define USER_CRYPTO_CRYPTO_H_

#include "stdint.h"

void secure_init(const uint8_t iv[16]);
void secure_decrypt_buffer(uint8_t *buf, uint32_t len);

#endif /* USER_CRYPTO_CRYPTO_H_ */
