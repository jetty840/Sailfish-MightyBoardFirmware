/*
 * T2-Timeout.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "Platform.hh"
#include "UART.hh"
#include "DebugPacketProcessor.hh"
#include <avr/interrupt.h>
#include "Timeout.hh"
#include "DebugPin.hh"
#if HAS_PSU
#include "PSU.hh"
#endif // HAS_PSU

#if defined(__AVR_ATmega644P__)
#define HAS_PASSTHRU 1
#elif defined(__AVR_ATmega168__)
#define HAS_PASSTHRU 0
#endif

int main() {
	TimeoutManager::init();
	uart[0].enable(true);
#if HAS_PASSTHRU
	uart[1].enable(true);
#endif // HAS_PASSTHRU
#if HAS_PSU
	initPsu();
#endif // HAS_PSU
	sei();
	while (1) {
		if (uart[0].in_.hasError() && uart[0].in_.getErrorCode() == PacketError::PACKET_TIMEOUT) {
			// nop for now
			uart[0].in_.reset();
		}
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
