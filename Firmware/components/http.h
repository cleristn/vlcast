#ifndef HTTP_H
#define HTTP_H
#include "esp_http_client.h"

typedef struct
{
    esp_http_client_config_t cfg;
    esp_http_client_handle_t cliente;
} ContextHttp;

void start_http(ContextHttp *ctx);
void send_http(const ContextHttp *ctx, char *buffer_saida, size_t tamanho_buffer, 
              uint8_t id, uint8_t chk, 
              const unsigned char *sig, size_t sig_len);
void clean_up_http(const ContextHttp *ctx);

#endif