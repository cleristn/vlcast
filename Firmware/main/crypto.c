#include <string.h>
#include "esp_log.h"
#include "mbedtls/pk.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/sha256.h"
#include "mbedtls/error.h"
#include "crypto.h"

extern const uint8_t private_key_pem_start[] asm("_binary_private_key_pem_start");
extern const uint8_t private_key_pem_end[] asm("_binary_private_key_pem_end");

static const char *TAG = "CRYPTO";

unsigned char signature_output[MBEDTLS_ECDSA_MAX_LEN];
size_t signature_len = 0;

// Assina o token e o checksum
void sign_token(uint8_t token, uint8_t checksum)
{
    int ret;
    mbedtls_pk_context pk;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    // Inicializa tudo
    mbedtls_pk_init(&pk);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    // Configura o RNG (Obrigatório fazer ANTES de ler a chave agora)
    const char *pers = "assinatura_esp32";
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *)pers, strlen(pers))) != 0)
    {
        ESP_LOGE(TAG, "Falha no seed RNG: -0x%04x", -ret);
        goto cleanup;
    }

    // Lê a Chave Privada
    if ((ret = mbedtls_pk_parse_key(&pk, private_key_pem_start,
                                    private_key_pem_end - private_key_pem_start, NULL, 0,
                                    mbedtls_ctr_drbg_random, &ctr_drbg)) != 0)
    {
        ESP_LOGE(TAG, "Falha ao ler chave privada: -0x%04x", -ret);
        goto cleanup;
    }

    // Cria o Hash (SHA-256)
    unsigned char hash[32];
    uint8_t data_to_sign[2] = {token, checksum};

    mbedtls_sha256(data_to_sign, sizeof(data_to_sign), hash, 0);

    // Assina o hash de 32 bytes obtidos atraves dos dois bytes
    if ((ret = mbedtls_pk_sign(&pk, MBEDTLS_MD_SHA256, hash, 0,
                               signature_output, sizeof(signature_output), &signature_len,
                               mbedtls_ctr_drbg_random, &ctr_drbg)) != 0)
    {
        ESP_LOGE(TAG, "Falha ao assinar: -0x%04x", -ret);
        goto cleanup;
    }

    ESP_LOGI(TAG, "Assinatura OK! Tamanho: %d bytes", signature_len);

    // Print HEX para debug
    printf("Signature Hex: ");
    for (int i = 0; i < signature_len; i++)
    {
        printf("%02x", signature_output[i]);
    }
    printf("\n");

cleanup:
    mbedtls_pk_free(&pk);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}