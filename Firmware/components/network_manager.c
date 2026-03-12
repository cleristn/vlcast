#include "processar_bit.h"
#include "http.h"
#include "crypto.h"

void send_signed_packaged_http(uint8_t token, uint8_t checksum_code, char *jsonBuffer, size_t size_buffer, ContextHttp *context)
{
    sign_token(token, checksum_code); // Assinar o token junto com o checksum

    if (signature_len > 0)
    {
        // Print para debug
        printf("Assinatura gerada: ");
        for (int i = 0; i < signature_len; i++)
        {
            printf("%02x", signature_output[i]);
        }
        printf("\n");

        // Nota: não envia o codigo do SOF
        send_http(
            context,          // Contexto do http
            jsonBuffer,       // Local de armazenamento do Token
            size_buffer,      // Tamanho (512)
            token,            // O Token (uint8_t)
            checksum_code,    // O Checksum (uint8_t)
            signature_output, // O array da assinatura
            signature_len     // O tamanho da assinatura
        );
    }
    else
    {
        printf("Erro ao gerar assinatura.\n");
    }
}