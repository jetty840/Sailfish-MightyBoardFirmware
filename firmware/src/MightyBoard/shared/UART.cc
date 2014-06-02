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
#include "EepromMap.hh"
#include "Eeprom.hh"
#include "Pin.hh"
#include <stdint.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/io.h>

// TODO: There should be a better way to enable this flag?
#if ASSERT_LINE_FIX
#include "ExtruderBoard.hh"
#endif

#if HAS_SLAVE_UART
// Avoid repeatedly creating temp objects
const Pin TX_Enable = TX_ENABLE_PIN;
const Pin RX_Enable = RX_ENABLE_PIN;
#endif

// We have to track the number of bytes that have been sent, so that we can
// filter
// them from our receive buffer later.This is only used for RS485 mode.
volatile uint8_t loopback_bytes = 0;

#if defined(ALTERNATE_UART) && HAS_SLAVE_UART != 0
#error Cannot use 2nd UART for both HAS_SLAVE_UART and ALTERNATE_UART
#endif

// We support three platforms: Atmega168 (1 UART), Atmega644, and
// Atmega1280/2560
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) ||                \
    defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1280__) ||              \
    defined(__AVR_ATmega2560__)
#else
#error UART not implemented on this processor type!
#endif

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__)

#define UBRR_VALUE 25
#define UCSR0A_VALUE 0

#define INIT_SERIAL(uart_)                                                     \
  {                                                                            \
    UBRR0H = UBRR_VALUE >> 8;                                                  \
    UBRR0L = UBRR_VALUE & 0xff;                                                \
                                                                               \
    /* set config for uart, explicitly clear TX interrupt flag */              \
    UCSR0A = UCSR0A_VALUE | _BV(TXC0);                                         \
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);                                          \
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);                                        \
  }

#elif defined(__AVR_ATmega644P__)

#define UBRR_VALUE 25
#define UBRRA_VALUE 0

// Adapted from ancient arduino/wiring rabbit hole
#define INIT_SERIAL(uart_)                                                     \
  {                                                                            \
    UBRR##uart_##H = UBRR_VALUE >> 8;                                          \
    UBRR##uart_##L = UBRR_VALUE & 0xff;                                        \
                                                                               \
    /* set config for uart_ */                                                 \
    UCSR##uart_##A = UBRRA_VALUE;                                              \
    UCSR##uart_##B = _BV(RXEN##uart_) | _BV(TXEN##uart_);                      \
    UCSR##uart_##C = _BV(UCSZ##uart_##1) | _BV(UCSZ##uart_##0);                \
  }

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

// Use double-speed mode for more accurate baud rate?
#define UBRR0_VALUE 16 // 115200 baud
#ifdef ALTERNATE_UART
// Alternate UART uses 115200 on both UARTS
#define UBRR1_VALUE 16 // 115200 baud
#else
// SLAVE_UART uses 38400 for the RS485 bus
#define UBRR1_VALUE 51 // 38400 baud
#endif
#define UCSRA_VALUE(uart_) _BV(U2X##uart_)

// Adapted from ancient arduino/wiring rabbit hole
#define INIT_SERIAL(uart_)                                                     \
  {                                                                            \
    UBRR##uart_##H = UBRR##uart_##_VALUE >> 8;                                 \
    UBRR##uart_##L = UBRR##uart_##_VALUE & 0xff;                               \
                                                                               \
    /* set config for uart_ */                                                 \
    UCSR##uart_##A = UCSRA_VALUE(uart_);                                       \
    UCSR##uart_##B = _BV(RXEN##uart_) | _BV(TXEN##uart_);                      \
    UCSR##uart_##C = _BV(UCSZ##uart_##1) | _BV(UCSZ##uart_##0);                \
  }
#endif

#define ENABLE_SERIAL_INTERRUPTS(uart_)                                        \
  { UCSR##uart_##B |= _BV(RXCIE##uart_) | _BV(TXCIE##uart_); }

#define DISABLE_SERIAL_INTERRUPTS(uart_)                                       \
  { UCSR##uart_##B &= ~(_BV(RXCIE##uart_) | _BV(TXCIE##uart_)); }

// TODO: Move these definitions to the board files, where they belong.
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__)

#ifdef ALTERNATE_UART
#error Alternate UART is not available on the ATmega328.
#endif

UART UART::hostUART(0, RS485);

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1280__) ||            \
    defined(__AVR_ATmega2560__)

UART UART::hostUART(0, RS232);

#if HAS_SLAVE_UART
UART UART::slaveUART(1, RS485);
#endif

#endif

void UART::init_serial() {
#ifdef ALTERNATE_UART
  // when using the ALTERNATE_UART option, both UARTS need to be initialized
  INIT_SERIAL(0);
  INIT_SERIAL(1);
#else
  if (index_ == 0) {
    INIT_SERIAL(0);
  }
#endif
#if HAS_SLAVE_UART	 
	else {
    INIT_SERIAL(1);
  }
#endif
}

void UART::send_byte(char data) {
  if (index_ == 0) {
    UDR0 = data;
  } 
#if HAS_SLAVE_UART || defined(ALTERNATE_UART)
  else {
    UDR1 = data;
  }
#endif
}

#if HAS_SLAVE_UART
// Transition to a non-transmitting state. This is only used for RS485 mode.
inline void listen() {
  //        TX_Enable.setValue(false);
  TX_Enable.setValue(false);
}

// Transition to a transmitting state
inline void speak() { TX_Enable.setValue(true); }
#endif

UART::UART(uint8_t index, communication_mode mode)
    : index_(index), mode_(mode), enabled_(false) {
  init_serial();
#ifdef ALTERNATE_UART
  // Value in EEPROM is the UART index: 0 for UART0 (USB), 1 for UART1
  // any other value is ignored.
  setHardwareUART(eeprom::getEeprom8(eeprom_offsets::ENABLE_ALTERNATE_UART, 0));
#endif
}

#ifdef ALTERNATE_UART
void UART::setHardwareUART(uint8_t index) {

  // Do nothing if there is no change in UART index.
  if (index == index_ || index > 1)
    return;

  // Save the new UART index
  index_ = index;

  // If we don't have an enabled UART, the do nothing.
  // the ::enable() function will enable the proper UART interrupts
  if (!enabled_)
    return;

  // Enable/Disable the proper ISR routines
  if (index == 0) {
    DISABLE_SERIAL_INTERRUPTS(1);
    ENABLE_SERIAL_INTERRUPTS(0);
  } else {
    DISABLE_SERIAL_INTERRUPTS(0);
    ENABLE_SERIAL_INTERRUPTS(1);
  }
}
#endif

// Subsequent bytes will be triggered by the tx complete interrupt.
void UART::beginSend() {
  if (!enabled_) {
    return;
  }

#if HAS_SLAVE_UART
  if (mode_ == RS485) {
    speak();
    _delay_us(10);
    loopback_bytes = 1;
  }
#endif
  send_byte(out.getNextByteToSend());
}

void UART::enable(bool enabled) {
  enabled_ = enabled;
  if (index_ == 0) {
    if (enabled) {
      ENABLE_SERIAL_INTERRUPTS(0);
    } else {
      DISABLE_SERIAL_INTERRUPTS(0);
    }
  }
#if HAS_SLAVE_UART || defined(ALTERNATE_UART)
  else if (index_ == 1) {
    if (enabled) {
      ENABLE_SERIAL_INTERRUPTS(1);
    } else {
      DISABLE_SERIAL_INTERRUPTS(1);
    }
  }
#endif

#if HAS_SLAVE_UART
  if (mode_ == RS485) {
    // If this is an RS485 pin, set up the RX and TX enable control lines.
    TX_Enable.setDirection(true);
    RX_Enable.setDirection(true);
    RX_Enable.setValue(false); // Active low
    listen();

    loopback_bytes = 0;
  }
#endif
}

#if HAS_SLAVE_UART
// Reset the UART to a listening state.  This is important for
// RS485-based comms.
void UART::reset() {
  if (mode_ == RS485) {
    loopback_bytes = 0;
    listen();
  }
}
#endif

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__)

// Send and receive interrupts
ISR(USART_RX_vect) {
  static uint8_t byte_in;

  byte_in = UDR0;
  if (loopback_bytes > 0) {
    loopback_bytes--;
  } else {
    UART::getHostUART().in.processByte(byte_in);

// Workaround for buggy hardware: have slave hold line high.
#if ASSERT_LINE_FIX
    if (UART::getHostUART().in.isFinished() &&
        (UART::getHostUART().in.read8(0) ==
         ExtruderBoard::getBoard().getSlaveID())) {
      speak();
    }
#endif
  }
}

ISR(USART_TX_vect) {
  if (UART::getHostUART().out.isSending()) {
    loopback_bytes++;
    UDR0 = UART::getHostUART().out.getNextByteToSend();
  } else {
    listen();
  }
}

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1280__) ||            \
    defined(__AVR_ATmega2560__)

// Send and receive interrupts
ISR(USART0_RX_vect) { UART::getHostUART().in.processByte(UDR0); }

ISR(USART0_TX_vect) {
  if (UART::getHostUART().out.isSending()) {
    UDR0 = UART::getHostUART().out.getNextByteToSend();
  }
}

#ifdef ALTERNATE_UART
ISR(USART1_RX_vect) { UART::getHostUART().in.processByte(UDR1); }

ISR(USART1_TX_vect) {
  if (UART::getHostUART().out.isSending()) {
    UDR1 = UART::getHostUART().out.getNextByteToSend();
  }
}
#endif

#if HAS_SLAVE_UART
ISR(USART1_RX_vect) {
  static uint8_t byte_in;

  byte_in = UDR1;
  if (loopback_bytes > 0) {
    loopback_bytes--;
  } else {
    UART::getSlaveUART().in.processByte(byte_in);
  }
}

ISR(USART1_TX_vect) {
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
