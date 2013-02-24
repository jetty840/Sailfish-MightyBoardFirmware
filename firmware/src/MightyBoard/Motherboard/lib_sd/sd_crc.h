#ifndef _SD_CRC_H__
#define _SD_CRC_H__

#include <stdint.h>

uint8_t sd_crc7(const uint8_t *data, uint8_t len);
uint16_t sd_crc16(const uint8_t *data, uint16_t len);

#endif
