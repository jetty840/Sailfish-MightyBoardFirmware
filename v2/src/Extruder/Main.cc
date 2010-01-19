/*
 * T4-Commands.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "UART.hh"
#include "DebugPacketProcessor.hh"
#include "HostThread.hh"
#include <avr/interrupt.h>
#include "Timeout.hh"
#include "DebugPin.hh"
#include "Timers.hh"

void runHostSlice();

int main() {
	// Intialize various modules
	uart[0].enable(true);
	uart[0].in_.reset();
	startTimers();
	sei();
	setDebugLED(true);
	while (1) {
		// Host interaction thread.
		runHostSlice();
		// Temperature monitoring thread
		//runTempThread();
	}
	return 0;
}
