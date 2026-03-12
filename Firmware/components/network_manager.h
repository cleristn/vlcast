#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "processar_bit.h"

void send_signed_packaged_http(uint8_t token, uint8_t checksum_code, char *jsonBuffer, size_t size_buffer, ContextHttp *context);

#endif