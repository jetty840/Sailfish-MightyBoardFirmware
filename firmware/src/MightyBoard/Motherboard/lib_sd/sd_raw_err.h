#ifndef _SD_RAW_ERR_H__

#define _SD_RAW_ERR_H__

#define SDR_ERR_BAD          0x01
#define SDR_ERR_BADRESPONSE  0x02
#define SDR_ERR_CRC          0x03
#define SDR_ERR_EINVAL       0x04
#define SDR_ERR_LOCKED       0x05
#define SDR_ERR_NOCARD       0x06

// Anything related to degraded communications has the high bit set
#define SDR_ERR_COMMS        0x80
#define SDR_ERR_PATTERN      0x81
#define SDR_ERR_VOLTAGE      0x82

#ifdef __cplusplus
extern "C"
{
#endif

extern uint8_t sd_errno;

#ifdef __cplusplus
}
#endif

#endif
