#include <stdint.h>

extern unsigned long rs485_tx_count;
extern unsigned long rs485_rx_count;
extern unsigned long rs485_packet_count;
extern unsigned long rs485_loopback_fails;
extern unsigned long slave_crc_errors;
extern unsigned long slave_timeouts;

extern unsigned long serial_tx_count;
extern unsigned long serial_rx_count;
extern unsigned long serial_packet_count;

void rs485_tx(uint8_t b);
void serial_tx(uint8_t b);
