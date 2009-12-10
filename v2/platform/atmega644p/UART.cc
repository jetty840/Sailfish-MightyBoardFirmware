/*
 * UART.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */
#include "UART.hh"
#include "Atmega644pConfig.h"

// MEGA644P_DOUBLE_SPEED_MODE is 1 if USXn is 1.
#ifndef MEGA644P_DOUBLE_SPEED_MODE
#define MEGA644P_DOUBLE_SPEED_MODE 0
#endif

#if MEGA644P_DOUBLE_SPEED_MODE
#define UBRR_VALUE 51
#define UBRRA_VALUE _BV(U2X##uart)
#else
#define UBRR_VALUE 25
#define UBRRA_VALUE 0
#endif


/// Adapted from ancient arduino/wiring rabbit hole
#define INIT_SERIAL(uart_) \
{ \
    UBRR##uart_##H = UBRR_VALUE >> 8; \
    UBRR##uart_##L = UBRR_VALUE & 0xff; \
    \
    /* set config for uart_ */ \
    UCSR##uart_##A = UBRRA_VALUE; \
    UCSR##uart_##B = _BV(RXEN##uart_) | _BV(TXEN##uart_) |  _BV(RXCIE##uart_) | _BV(TXCIE##uart_); \
    UCSR##uart_##C = _BV(UCSZ##uart_##1)|_BV(UCSZ##uart_##0); \
    /* defaults to 8-bit, no parity, 1 stop bit */ \
}

UART uart[2] = {
		UART(0),
		UART(1)
};

UART::UART(uint8_t index) {
	if (index == 0) {
		INIT_SERIAL(0);
	} else if (index == 1) {
		INIT_SERIAL(1);
	}
}

void UART::beginSend() {

}

void UART::enable(bool enabled) {

}

extern UART uart[];

#endif // MB_UTIL_UART_HH_
