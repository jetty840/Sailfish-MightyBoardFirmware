/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "UART.hh"
#include <stdint.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "Configuration.hh"

// MEGA644P_DOUBLE_SPEED_MODE is 1 if USXn is 1.
#ifndef MEGA644P_DOUBLE_SPEED_MODE
#define MEGA644P_DOUBLE_SPEED_MODE 0
#endif

#if MEGA644P_DOUBLE_SPEED_MODE
#define UBRR_VALUE 51
#define UBRRA_VALUE _BV(U2X##uart_)
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

UART UART::uart[2] = {
		UART(0),
		UART(1)
};

volatile bool listening = true;

// Unlike the old implementation, we go half-duplex: we don't listen while sending.
inline void listen() {
	PORTD &= ~(_BV(4) | _BV(5) );
	// Turn on the receiver
	UCSR1B |= _BV(RXEN1);
}

inline void speak() {
	PORTD |= (_BV(4) | _BV(5));
	// Turn off the receiver
	UCSR1B &= ~_BV(RXEN1);
}

UART::UART(uint8_t index) : index_(index), enabled_(false) {
	if (index_ == 0) {
		INIT_SERIAL(0);
	} else if (index_ == 1) {
		INIT_SERIAL(1);
		// UART1 is an RS485 port, and requires additional setup.
		// Read enable: PD5, active low
		// Tx enable: PD4, active high
		DDRD |= _BV(5) | _BV(4);
		listen();
	}
}

#define SEND_BYTE(uart_,data_) UDR##uart_ = data_

/// Subsequent bytes will be triggered by the tx complete interrupt.
void UART::beginSend() {
	if (!enabled_) { return; }
	uint8_t send_byte = out.getNextByteToSend();
	if (index_ == 0) {
		SEND_BYTE(0,send_byte);
	} else if (index_ == 1) {
		speak();
		SEND_BYTE(1,send_byte);
	}
}

void UART::enable(bool enabled) {
	enabled_ = enabled;
	if (index_ == 0) {
		if (enabled) { ENABLE_SERIAL_INTERRUPTS(0); }
		else { DISABLE_SERIAL_INTERRUPTS(0); }
	} else if (index_ == 1) {
		if (enabled) { ENABLE_SERIAL_INTERRUPTS(1); }
		else { DISABLE_SERIAL_INTERRUPTS(1); }
	}
}

// Reset the UART to a listening state.  This is important for
// RS485-based comms.
void UART::reset() {
	if (index_ == 1) {
		listen();
	}
}

// Send and receive interrupts
#define UART_RX_ISR(uart_) \
ISR(USART##uart_##_RX_vect) \
{ \
	UART::uart[uart_].in.processByte( UDR##uart_ ); \
}

UART_RX_ISR(0);
UART_RX_ISR(1);

ISR(USART0_TX_vect)
{
	if (UART::uart[0].out.isSending()) {
		UDR0 = UART::uart[0].out.getNextByteToSend();
	}
}

ISR(USART1_TX_vect)
{
	if (UART::uart[1].out.isSending()) {
		UDR1 = UART::uart[1].out.getNextByteToSend();
	} else {
		listen();
	}
}

