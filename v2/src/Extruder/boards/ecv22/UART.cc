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
#include "Configuration.hh"
#include "ExtruderBoard.hh"
#include <stdint.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

// MEGA168_DOUBLE_SPEED_MODE is 1 if USXn is 1.
#ifndef MEGA168_DOUBLE_SPEED_MODE
#define MEGA168_DOUBLE_SPEED_MODE 0
#endif

#if MEGA168_DOUBLE_SPEED_MODE
#define UBRR_VALUE 51
#define UCSR0A_VALUE _BV(U2X0)
#else
#define UBRR_VALUE 25
#define UCSR0A_VALUE 0
#endif

UART UART::uart;

volatile uint8_t loopback_bytes = 0;

// Unlike the old implementation, we go half-duplex: we don't listen while sending, and vice versa.
inline void speak() {
	TX_ENABLE_PIN.setValue(true);
}

inline void listen() {
	TX_ENABLE_PIN.setValue(false);
}

UART::UART() : enabled(false) {
    UBRR0H = UBRR_VALUE >> 8;
    UBRR0L = UBRR_VALUE & 0xff;
    /* set config for uart, explicitly clear TX interrupt flag */
    UCSR0A = UCSR0A_VALUE | _BV(TXC0);
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    UCSR0C = _BV(UCSZ01)|_BV(UCSZ00);
    /* defaults to 8-bit, no parity, 1 stop bit */
    TX_ENABLE_PIN.setDirection(true);
    RX_ENABLE_PIN.setDirection(true);
	RX_ENABLE_PIN.setValue(false); // Always in listen mode

    // pulup on RX
    Pin rxPin(PortD,0);
    rxPin.setDirection(false);
    rxPin.setValue(true);


    listen();
}

// Reset the UART to a listening state.  This is important for
// RS485-based comms.  This call resets the loopback count, and
// so should only be called after a timeout or read failure.
void UART::reset() {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		loopback_bytes = 0;
		listen();
	}
}

/// Subsequent bytes will be triggered by the tx complete interrupt.
void UART::beginSend() {
	if (!enabled) { return; }
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		uint8_t send_byte = out.getNextByteToSend();
		speak();
		UDR0 = send_byte;
		loopback_bytes = 1;
	}
}

void UART::enable(bool enabled_in) {
	enabled = enabled_in;
	if (enabled) {
		UCSR0B |=  _BV(RXCIE0) | _BV(TXCIE0);
	} else {
		UCSR0B &= ~(_BV(RXCIE0) | _BV(TXCIE0));
	}
}

volatile uint8_t in_byte;
// Send and receive interrupts
ISR(USART_RX_vect)
{
	in_byte = UDR0;
	if (loopback_bytes > 0) {
		loopback_bytes--;
	} else {
		UART::getHostUART().in.processByte( in_byte );
	}
}

ISR(USART_TX_vect)
{
	if (UART::getHostUART().out.isSending()) {
		UDR0 = UART::getHostUART().out.getNextByteToSend();
		loopback_bytes++;
	} else {
		listen();
	}
}

