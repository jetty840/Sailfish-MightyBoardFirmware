/*
 * T4-Commands.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "UART.hh"
#include "PSU.hh"
#include "DebugPacketProcessor.hh"
#include "Host.hh"
#include "Tool.hh"
#include "Command.hh"
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "Timeout.hh"
#include "DebugPin.hh"
#include "Steppers.hh"
#include "Timers.hh"

void reset() {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		steppers.abort();
		command::reset();
	}
}

int main() {
	//
	psu::init();
	// Intialize various modules
	uart[0].enable(true);
	uart[0].in_.reset();
	uart[1].enable(true);
	uart[1].in_.reset();
	sei();
	//steppers.setTarget(points[0],500);
	int point_idx = 1;
	startTimers();
	setDebugLED(true);
	while (1) {
		// Toolhead interaction thread.
		tool::runToolSlice();
		// Host interaction thread.
		runHostSlice();
		// SD command buffer read/refill thread.
		//runSDSlice();
		// Command handling thread.
		command::runCommandSlice();
	}
	return 0;
}
