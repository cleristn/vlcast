#ifndef PROCESSAR_BIT_H
#define PROCESSAR_BIT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct 
{
    bool sof_found;
    uint8_t shiftReg;
    uint8_t token;
    uint8_t checksum_code;
    int count_bits_token;
    int count_bits_checksum;
}ProtocoloCtx;

bool process_bit(ProtocoloCtx *ctx, int bit);

#endif