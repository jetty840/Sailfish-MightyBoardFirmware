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

// MEGA168_DOUBLE_SPEED_MODE is 1 if USXn is 1.
#ifndef MEGA168_DOUBLE_SPEED_MODE
#define MEGA168_DOUBLE_SPEED_MODE 0
#endif

#if MEGA168_DOUBLE_SPEED_MODE
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


// Unlike the old implementation, we go half-duplex: we don't listen while sending.
inline void listen() {
	PORTD &= ~_BV(4);
	PORTC &= ~_BV(2);
}

inline void speak() {
	PORTD |= _BV(4);
	PORTC |= _BV(2);
}

UART::UART(uint8_t index) : index_(index), enabled_(false) {
	INIT_SERIAL(0);
	DDRD |= _BV(4);
	DDRC |= _BV(2);
	listen();
}

#define SEND_BYTE(uart_,data_) UDR##uart_ = data_

/// Subsequent bytes will be triggered by the tx complete interrupt.
void UART::beginSend() {
	if (!enabled_) { return; }
	uint8_t send_byte = out_.getNextByteToSend();
	speak();
	SEND_BYTE(0,send_byte);
}

void UART::enable(bool enabled) {
	enabled_ = enabled;
	if (enabled) { ENABLE_SERIAL_INTERRUPTS(0); }
	else { DISABLE_SERIAL_INTERRUPTS(0); }
}

// Send and receive interrupts
ISR(USART_RX_vect)
{
	uart[0].in_.processByte( UDR0 );
}

ISR(USART_TX_vect)
{
	if (uart[0].out_.isSending()) {
		UDR0 = uart[0].out_.getNextByteToSend();
	} else {
		listen();
	}
}

