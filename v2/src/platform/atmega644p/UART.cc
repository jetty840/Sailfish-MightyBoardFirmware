/*
 * UART.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */
#include "util/UART.hh"
#include <stdint.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/io.h>

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

UART::UART(uint8_t index) : index_(index), enabled_(false) {
	if (index_ == 0) {
		INIT_SERIAL(0);
	} else if (index_ == 1) {
		INIT_SERIAL(1);
	}
}

#define SEND_BYTE(uart_,data_) UDR##uart_ = data_

/// Subsequent bytes will be triggered by the tx complete interrupt.
void UART::beginSend() {
	if (!enabled_) { return; }
	uint8_t send_byte = out_.getNextByteToSend();
	if (index_ == 0) {
		SEND_BYTE(0,send_byte);
	} else if (index_ == 1) {
		SEND_BYTE(1,send_byte);
	}
}

void UART::enable(bool enabled) {
	enabled_ = enabled;
}

// Send and receive interrupts
#define UART_RX_ISR(uart_) \
ISR(USART##uart_##_RX_vect) \
{ \
	uart[uart_].in_.processByte( UDR##uart_ ); \
}

#define UART_TX_ISR(uart_) \
ISR(USART##uart_##_TX_vect) \
{ \
	if (uart[uart_].out_.isSending()) { \
		UDR##uart_ = uart[uart_].out_.getNextByteToSend(); \
	} \
}

UART_RX_ISR(0);
UART_RX_ISR(1);

UART_TX_ISR(0);
UART_TX_ISR(1);

