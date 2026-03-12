#include "checksum_8.h"

uint8_t calculate_crc8(uint8_t *data, uint8_t length)
{
    uint8_t crc = 0x00;

    for (uint8_t i = 0; i < length; i++)
    {
        crc ^= data[i]; // Aplica o byte atual ao CRC via XOR

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80) // Verifica se o bit mais significativo for 1
            {
                crc = (crc << 1) ^ 0x07; // Desloca e aplica o polinômio (0x07)
            }
            else
            {
                crc <<= 1; // Apenas desloca
            }
        }
    }
    return crc;
}