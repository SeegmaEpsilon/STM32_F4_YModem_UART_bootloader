#include "crypto.h"
#include "aes.h"
#include "key_template.h"
#include <string.h>

static struct AES_ctx g_aes;
static uint8_t g_ks[AES_BLOCKLEN];      // keystream block
static uint8_t g_ks_pos = AES_BLOCKLEN; // 16 => нужно сгенерить новый блок

static inline void iv_inc_be(uint8_t iv[AES_BLOCKLEN])
{
    // big-endian: Iv[15]++ с переносом
    for (int i = AES_BLOCKLEN - 1; i >= 0; --i) {
        if (++iv[i] != 0) break;
    }
}

void secure_init(const uint8_t iv[16])
{
    AES_init_ctx_iv(&g_aes, aes_key, iv);   // кладёт IV в g_aes.Iv, раунд-ключи в RoundKey
    g_ks_pos = AES_BLOCKLEN;                // сброс позиции потока
}

void secure_decrypt_buffer(uint8_t *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; ++i)
    {
        if (g_ks_pos == AES_BLOCKLEN)
        {
            memcpy(g_ks, g_aes.Iv, AES_BLOCKLEN);
            AES_ECB_encrypt(&g_aes, g_ks);  // keystream = AES(key, IV)
            iv_inc_be(g_aes.Iv);            // следующий блок
            g_ks_pos = 0;
        }
        buf[i] ^= g_ks[g_ks_pos++];
    }
}
