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
#include <stdint.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>


// We support three platforms: Atmega168 (1 UART), Atmega644, and Atmega1280/2560


#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328__)

    // Actually we don't support this yet.
    #error ATmega168/328 target not supported!

#elif defined (__AVR_ATmega644P__)

    #define UBRR_VALUE 25
    #define UBRRA_VALUE 0

    // Adapted from ancient arduino/wiring rabbit hole
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

#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)

    #define UBRR0_VALUE 8 // 115200
    #define UBRR1_VALUE 25 // 38400 baud
    #define UCSRA_VALUE(uart_) 0

    // Adapted from ancient arduino/wiring rabbit hole
    #define INIT_SERIAL(uart_) \
    { \
    UBRR##uart_##H = UBRR##uart_##_VALUE >> 8; \
    UBRR##uart_##L = UBRR##uart_##_VALUE & 0xff; \
    \
    /* set config for uart_ */ \
    UCSR##uart_##A = UCSRA_VALUE(uart_); \
    UCSR##uart_##B = _BV(RXEN##uart_) | _BV(TXEN##uart_); \
    UCSR##uart_##C = _BV(UCSZ##uart_##1)|_BV(UCSZ##uart_##0); \
    /* defaults to 8-bit, no parity, 1 stop bit */ \
    }

#endif

#define SEND_BYTE(uart_,data_) UDR##uart_ = data_

#define ENABLE_SERIAL_INTERRUPTS(uart_) \
{ \
UCSR##uart_##B |= _BV(RXCIE##uart_) | _BV(TXCIE##uart_); \
}

#define DISABLE_SERIAL_INTERRUPTS(uart_) \
{ \
UCSR##uart_##B &= ~(_BV(RXCIE##uart_) | _BV(TXCIE##uart_)); \
}

UART UART::hostUART(0, RS232);
UART UART::slaveUART(1, RS485);

// We have to track the number of bytes that have been sent, so that we can filter
// them from our receive buffer later.This is only used for RS485 mode.
volatile uint8_t loopback_bytes = 0;

// Transition to a non-transmitting state. This is only used for RS485 mode.
inline void listen() {
	TX_ENABLE_PIN.setValue(false);
}

// Transition to a transmitting state
inline void speak() {
	TX_ENABLE_PIN.setValue(true);
}

UART::UART(uint8_t index, communication_mode mode) :
    index_(index),
    mode_ (mode),
    enabled_(false) {
        if (mode_ == RS232) {
                INIT_SERIAL(0);
        } else if (mode_ == RS485) {
                INIT_SERIAL(1);
		// UART1 is an RS485 port, and requires additional setup.
		// Read enable: PD5, active low
		// Tx enable: PD4, active high
		TX_ENABLE_PIN.setDirection(true);
		RX_ENABLE_PIN.setDirection(true);
		RX_ENABLE_PIN.setValue(false);  // Active low

                loopback_bytes = 0;
		listen();
	}
}

/// Subsequent bytes will be triggered by the tx complete interrupt.
void UART::beginSend() {
	if (!enabled_) { return; }
	uint8_t send_byte = out.getNextByteToSend();
	if (index_ == 0) {
		SEND_BYTE(0,send_byte);
	} else if (index_ == 1) {
		speak();
		_delay_us(10);
		loopback_bytes = 1;
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
        if (mode_ == RS485) {
		loopback_bytes = 0;
		listen();
	}
}

// Send and receive interrupts
// Note: These need to be changed if the hardware serial port changes.
ISR(USART0_RX_vect)
{
        UART::getHostUART().in.processByte( UDR0 );
}

ISR(USART0_TX_vect)
{
        if (UART::getHostUART().out.isSending()) {
                UDR0 = UART::getHostUART().out.getNextByteToSend();
	}
}

ISR(USART1_RX_vect)
{
        static uint8_t byte_in;

        byte_in = UDR1;
        if (loopback_bytes > 0) {
                loopback_bytes--;
        } else {
                UART::getSlaveUART().in.processByte( byte_in );
        }
}

ISR(USART1_TX_vect)
{
        if (UART::getSlaveUART().out.isSending()) {
		loopback_bytes++;
                UDR1 = UART::getSlaveUART().out.getNextByteToSend();
	} else {
		_delay_us(10);
		listen();
	}
}

