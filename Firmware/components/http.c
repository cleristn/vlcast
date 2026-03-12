#include "http.h"
#include "sdkconfig.h"

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        break;
    case HTTP_EVENT_ON_DATA:
        break;
    default:
        break;
    }
    return ESP_OK;
}

void start_http(ContextHttp *ctx)
{
    const char *full_url = "http://" CONFIG_MY_IP ":3000/verify";

    ctx->cfg = (esp_http_client_config_t){
        .url = full_url,
        .method = HTTP_METHOD_POST,
        .event_handler = _http_event_handler,
        .keep_alive_enable = false,
        .timeout_ms = 5000,
    };
    ctx->cliente = esp_http_client_init(&ctx->cfg);
    ESP_ERROR_CHECK(esp_http_client_set_header(ctx->cliente, "Connection", "close"));
}

void send_http(const ContextHttp *ctx, char *buffer_saida, size_t tamanho_buffer,
              uint8_t id, uint8_t chk,
              const unsigned char *sig, size_t sig_len)
{
    // Usar Base64 futurarmente para reduzir tamanho enviado
    //TO DO
    // Colocar tamaho fixo no define e usar ele no snprinf tambem
    char sig_hex[160] = {0};
    int pos = 0;

    // Armazena 
    for (int i = 0; i < sig_len; i++)
    {
        // %02x converte um byte (ex: 10) para hex ("0a")
        // O += vai acumulando na string
        pos += sprintf(&sig_hex[pos], "%02x", sig[i]);
        // seguro pos += snprintf(&sig_hex[pos], tamanho_restante, "%02x", sig[i]);
    }

    // Montar o JSON Final
    // Estrutura: {"tok": 200, "chs": 5, "sig": "a1b2..."}
    snprintf(
        buffer_saida,
        tamanho_buffer,
        "{\"tok\":%d,\"chs\":%d,\"sig\":\"%s\"}",
        id,
        chk,
        sig_hex);
    esp_err_t ero = ESP_FAIL;

    if (ctx->cliente != NULL && buffer_saida != NULL)
    {
        esp_http_client_set_header(ctx->cliente, "Content-Type", "application/json");
        // Define o corpo do POST usando o buffer
        ESP_ERROR_CHECK(esp_http_client_set_post_field(ctx->cliente, buffer_saida, strlen(buffer_saida)));
        esp_http_client_set_method(ctx->cliente, HTTP_METHOD_POST);
        // Dispara o envio
        ero = esp_http_client_perform(ctx->cliente);
    }
    else
    {
        printf("Algum dado é nulo (Cliente HTTP ou Buffer).\n");
    }

    // Validação do resultado
    if (ero == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(ctx->cliente);
        printf("ESP_OK - HTTP STATUS CODE: %i\n", status_code);
    }
    else
    {
        printf("Falha na requisição: %s\n", esp_err_to_name(ero));
    }
}

void clean_up_http(const ContextHttp *ctx)
{
    esp_http_client_cleanup(ctx->cliente);
}