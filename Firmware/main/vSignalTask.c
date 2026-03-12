#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

#include "http.h"
#include "processar_bit.h"
#include "network_manager.h"

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define ADC_UNIT ADC_UNIT_1
#define ADC_CHANNEL ADC_CHANNEL_3
#define GPIO_SENSOR GPIO_NUM_4 // Pino da GPIO

#define CHARGE_TIME_US 100 // 400 charge time junto com 100 é o nipe
#define DRAIN_TIME_US 31000

#define CODE_TOGGLE_DURATION_MS 150 // Duracao de uma piscada
#define SAMPLE_LEN_US 16677         // Duracao da coleta de amostras

#define JSON_BUFFER_LEN 512 // Tamanho do buffer enviado via http

bool sof_found = false;
uint8_t shiftReg = 0;
uint8_t sessionId = 0;
uint8_t checksumCode = 0;
int count_bits_token = 0;
int count_bits_checksum = 0;

// Inicia configuracao do ADC para leitura do LED
// Processa todo sinal recebido
void vSignalTask(void *pvParameters)
{
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {.unit_id = ADC_UNIT, .ulp_mode = ADC_ULP_MODE_DISABLE};
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {.bitwidth = ADC_BITWIDTH_DEFAULT, .atten = ADC_ATTEN_DB_12};
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &config));

    // Recebe o contexto do http
    ContextHttp *my_context_http = (ContextHttp *)pvParameters;

    int current_status = 0;
    int read_status = 0;

    int amplified_raw_signal = 0;
    int trigger = 0;
    int reset = 0;

    float average_raw = 0;
    float alpha = 0;
    float average_amplified_signal = 0;
    int64_t start_time = 0;
    int64_t elapsed_time_us = 0;

    char jsonBuffer[JSON_BUFFER_LEN];

    ProtocoloCtx process_bit_t = {
        .checksum_code = checksumCode,
        .count_bits_token = count_bits_token,
        .count_bits_checksum = count_bits_checksum,
        .sof_found = sof_found,
        .token = sessionId,
        .shiftReg = shiftReg};

    printf("--- INICIANDO LEITURA FIXA ---\n");

    printf("Wifi Conectado\n");
    while (1)
    {
        // LEITURA DOS SINAIS
        gpio_set_direction(GPIO_SENSOR, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_SENSOR, 1);
        esp_rom_delay_us(CHARGE_TIME_US);

        gpio_set_direction(GPIO_SENSOR, GPIO_MODE_INPUT);
        esp_rom_delay_us(DRAIN_TIME_US);

        uint32_t sample_sum = 0;
        int sample = 0;
        int raw_reading = 0;

        int64_t inicio = esp_timer_get_time();

        // Loop para armazenar as amostras
        while ((esp_timer_get_time() - inicio) < SAMPLE_LEN_US)
        {
            adc_oneshot_read(adc1_handle, ADC_CHANNEL, &raw_reading);
            sample_sum += raw_reading;
            sample++;
            esp_rom_delay_us(2); // Nota: este tempo de delay quase nao influencia
        }

        int final_raw = 0;

        if (sample > 0)
            final_raw = sample_sum / sample;

        // Se o raw atual tiver uma diferenca maior que 190 do valor anterior da media
        // altera os pesos para a media se movimentar rapidamente e acompanhar o sinal
        if ((abs((int)(final_raw - average_raw))) > 190)
        {
            alpha = 0.8f;
        }
        else // Suaviza a media
        {
            alpha = 0.2f;
        }

        // Calcula media movel exponencial
        average_raw = (average_raw * (1 - alpha) + final_raw * alpha);

        amplified_raw_signal = (average_raw - final_raw) * 15;
        average_amplified_signal = (average_amplified_signal * 0.99f + amplified_raw_signal * 0.01f);

        // Pontos que definem a mudanca de bit
        trigger = average_amplified_signal - 100;
        reset = average_amplified_signal + 100;

        if (amplified_raw_signal < trigger)
        {
            current_status = 0;
        }

        if (amplified_raw_signal > reset)
        {
            current_status = 1;
        }

        if (read_status != current_status)
        {
            // Calcula o tempo decorrido em um estado apenas
            elapsed_time_us = esp_timer_get_time() - start_time;
            int64_t elapsed_time_ms = elapsed_time_us / 1000;
            // printf("//Tempo no estado %i: %lli\n", read_status, elapsed_time_ms);

            // Armazena o arredondamento da divisão do tempo decorrido com o tempo de duracao do bit
            // Arredonda para o inteiro mais proximo
            int n = (elapsed_time_ms + CODE_TOGGLE_DURATION_MS / 2) / CODE_TOGGLE_DURATION_MS;

            // Itera a quantidade de bits identificado
            for (int i = 0; i < n; i++)
            {
                // Chama a função para armazenar os bits
                bool finished_package = process_bit(&process_bit_t, read_status);

                if (finished_package)
                {
                    send_signed_packaged_http(process_bit_t.token, process_bit_t.checksum_code, jsonBuffer, sizeof(jsonBuffer), my_context_http);
                }
            }

            read_status = current_status;
            start_time = esp_timer_get_time();
        }
        // Print para rodar no SerialPloter
        printf("$%d,%d,%d,%d,%f\n", reset, amplified_raw_signal, trigger, current_status * 500, average_amplified_signal);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
