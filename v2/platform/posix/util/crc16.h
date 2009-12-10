#ifndef MB_PLATFORM_POSIX_UTIL_CRC16_H_
#define MB_PLATFORM_POSIX_UTIL_CRC16_H_

/*
 * crc16.h
 *
 *  Created on: Dec 8, 2009
 *      Author: phooky
 */
#include <stdint.h>

static __inline__ uint8_t _crc_ibutton_update (
		uint8_t crc,
		uint8_t data
) {
    uint8_t i;

       crc = crc ^ data;
       for (i = 0; i < 8; i++)
       {
           if (crc & 0x01)
               crc = (crc >> 1) ^ 0x8C;
           else
               crc >>= 1;
       }

       return crc;
}

#endif // MB_PLATFORM_POSIX_UTIL_CRC16_H_
