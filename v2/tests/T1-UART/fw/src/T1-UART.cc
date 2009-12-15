/*
 * T1-UART.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "util/UART.hh"
#include "util/DebugPacketProcessor.hh"
#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#if defined(__AVR_ATmega644P__)
#define DEBUG_PIN 0
#elif defined(__AVR_ATmega168__)
#define DEBUG_PIN 5
#endif

int main() {
	DDRB = (DDRB & ~_BV(DEBUG_PIN)) |_BV(DEBUG_PIN);
	PORTB = _BV(DEBUG_PIN);
	uart[0].enable(true);
	sei();
	while (1) {
		if (uart[0].in_.isFinished()) {
			if (processDebugPacket(uart[0].in_, uart[0].out_)) {
				uart[0].in_.reset();
				uart[0].beginSend();
			} else {
				uart[0].in_.reset();
			}
			while (!uart[0].out_.isFinished()) {}

			uart[0].out_.reset();
		}
	}
	return 0;
}
