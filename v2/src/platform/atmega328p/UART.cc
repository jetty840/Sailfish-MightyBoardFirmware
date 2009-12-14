/*
 * UART.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */
#include "util/UART.hh"
#include <Platform.hh>
#include <stdint.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/io.h>

// MEGA644P_DOUBLE_SPEED_MODE is 1 if USXn is 1.
#ifndef MEGA328_DOUBLE_SPEED_MODE
#define MEGA328_DOUBLE_SPEED_MODE 0
#endif

#if MEGA329_DOUBLE_SPEED_MODE
#define UBRR_VALUE 51
#define UBRRA_VALUE _BV(U2X0)
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
    UCSR##uart_##B = _BV(RXEN##uart_) | _BV(TXEN##uart_); \
    UCSR##uart_##C = _BV(UCSZ##uart_##1)|_BV(UCSZ##uart_##0); \
    /* defaults to 8-bit, no parity, 1 stop bit */ \
}

#define ENABLE_SERIAL_INTERRUPTS(uart_) \
{ \
	UCSR##uart_##B |=  _BV(RXCIE##uart_) | _BV(TXCIE##uart_); \
}

#define DISABLE_SERIAL_INTERRUPTS(uart_) \
{ \
	UCSR##uart_##B &= ~(_BV(RXCIE##uart_) | _BV(TXCIE##uart_)); \
}

UART uart[UART_COUNT] = {
		UART(0)
};

UART::UART(uint8_t index) : index_(index), enabled_(false) {
	INIT_SERIAL(0);
}

#define SEND_BYTE(uart_,data_) UDR##uart_ = data_

/// Subsequent bytes will be triggered by the tx complete interrupt.
void UART::beginSend() {
	if (!enabled_) { return; }
	uint8_t send_byte = out_.getNextByteToSend();
	SEND_BYTE(0,send_byte);
}

void UART::enable(bool enabled) {
	enabled_ = enabled;
	if (enabled) { ENABLE_SERIAL_INTERRUPTS(0); }
	else { DISABLE_SERIAL_INTERRUPTS(0); }
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
UART_TX_ISR(0);

