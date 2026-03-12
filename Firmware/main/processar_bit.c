#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "checksum_8.h"
#include <stdbool.h>
#include "processar_bit.h"

// Verifica o SOF, e armazena os bits seguintes que representa o Token e CRC
bool process_bit(ProtocoloCtx *ctx, int bit)
{
    if (!ctx->sof_found)
    {
        ctx->shiftReg = (ctx->shiftReg << 1) | bit;
        // Verifica se achou o padrão 01011 (0x0B)
        if ((ctx->shiftReg & 0x1F) == 0x0B)
        {
            ctx->sof_found = true;
            ctx->token = 0; // Limpa para começar a gravar
            ctx->checksum_code = 0;
            ctx->count_bits_token = 0;
            ctx->count_bits_checksum = 0;
        }
    }
    // Armazena Token apos SOF encontrado
    else if (ctx->count_bits_token < 8)
    {
        ctx->token = (ctx->token << 1) | bit;
        ctx->count_bits_token++;
    }
    // Armazena CRC apos token armazenado
    else if (ctx->count_bits_checksum < 8)
    {
        ctx->checksum_code = (ctx->checksum_code << 1) | bit;
        ctx->count_bits_checksum++;

        // Se acabou de pegar o ultimo bit do CRC
        if (ctx->count_bits_checksum == 8)
        {
            ctx->sof_found = false;
            // Verifica se checksum e valido
            if (calculate_crc8(&ctx->token, sizeof(ctx->token)) == ctx->checksum_code)
            {
                return true;
            }
        }
    }
    return false;
}
