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
#include <util/delay.h>

// We support three platforms: Atmega168 (1 UART), Atmega644, and Atmega1280/2560


#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328__)

    #define UBRR_VALUE 25
    #define UCSR0A_VALUE 0

    #define INIT_SERIAL(uart_) \
    { \
        UBRR0H = UBRR_VALUE >> 8; \
        UBRR0L = UBRR_VALUE & 0xff; \
        \
        /* set config for uart, explicitly clear TX interrupt flag */ \
        UCSR0A = UCSR0A_VALUE | _BV(TXC0); \
        UCSR0B = _BV(RXEN0) | _BV(TXEN0); \
        UCSR0C = _BV(UCSZ01)|_BV(UCSZ00); \
    }

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
    }

#endif

#define ENABLE_SERIAL_INTERRUPTS(uart_) \
{ \
UCSR##uart_##B |= _BV(RXCIE##uart_) | _BV(TXCIE##uart_); \
}

#define DISABLE_SERIAL_INTERRUPTS(uart_) \
{ \
UCSR##uart_##B &= ~(_BV(RXCIE##uart_) | _BV(TXCIE##uart_)); \
}

// TODO: Move these definitions to the board files, where they belong.
// TODO: Also need to make 0 an RS485 on the extruder.
UART UART::hostUART(0, RS232);
#if HAS_SLAVE_UART
UART UART::slaveUART(1, RS485);
#endif


void UART::init_serial() {
    if(index_ == 0) {
        INIT_SERIAL(0);
    }
#if HAS_SLAVE_UART
    else {
        INIT_SERIAL(1);
    }
#endif
}

void UART::send_byte(char data) {
    if(index_ == 0) {
        UDR0 = data;
    }
#if HAS_SLAVE_UART
    else {
        UDR1 = data;
    }
#endif
}


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

        init_serial();

        if (mode_ == RS485) {
		// UART1 is an RS485 port, and requires additional setup.
		// Read enable: PD5, active low
		// Tx enable: PD4, active high
		TX_ENABLE_PIN.setDirection(true);
		RX_ENABLE_PIN.setDirection(true);
		RX_ENABLE_PIN.setValue(false);  // Active low

                // TODO: Should set pullup on RX?
//                Pin rxPin(PortD,0);
//                rxPin.setDirection(false);
//                rxPin.setValue(true);

                loopback_bytes = 0;
		listen();
	}
}

/// Subsequent bytes will be triggered by the tx complete interrupt.
void UART::beginSend() {
	if (!enabled_) { return; }

        if (mode_ == RS485) {
		speak();
		_delay_us(10);
		loopback_bytes = 1;
	}

        send_byte(out.getNextByteToSend());
}

void UART::enable(bool enabled) {
	enabled_ = enabled;
	if (index_ == 0) {
                if (enabled) { ENABLE_SERIAL_INTERRUPTS(0); }
		else { DISABLE_SERIAL_INTERRUPTS(0); }
        }
#if HAS_SLAVE_UART
        else if (index_ == 1) {
		if (enabled) { ENABLE_SERIAL_INTERRUPTS(1); }
		else { DISABLE_SERIAL_INTERRUPTS(1); }
	}
#endif
}

// Reset the UART to a listening state.  This is important for
// RS485-based comms.
void UART::reset() {
        if (mode_ == RS485) {
                loopback_bytes = 0;
                listen();
	}
}

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328__)

    // Send and receive interrupts
    ISR(USART_RX_vect)
    {
            static uint8_t byte_in;

            byte_in = UDR0;
            if (loopback_bytes > 0) {
                    loopback_bytes--;
            } else {
                    UART::getHostUART().in.processByte( byte_in );

                    // Workaround for buggy hardware: have slave hold line high.
    #ifdef ASSERT_LINE_FIX
                    if (UART::getHostUART().in.isFinished()) {
                        speak();
                    }
    #endif
            }
    }

    ISR(USART_TX_vect)
    {
            if (UART::getHostUART().out.isSending()) {
                    loopback_bytes++;
                    UDR0 = UART::getHostUART().out.getNextByteToSend();
            } else {
                    listen();
            }
    }

#elif defined (__AVR_ATmega644P__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)

    // Send and receive interrupts
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

    #if HAS_SLAVE_UART
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
    #endif

#endif
