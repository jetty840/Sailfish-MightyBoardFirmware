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

void runHostSlice();

int main() {
	// Intialize various modules
	uart[0].enable(true);
	uart[0].in_.reset();
	sei();
	//steppers.setTarget(points[0],500);
	int point_idx = 1;
	setDebugLED(true);
	while (1) {
		// Host interaction thread.
		runHostSlice();
		// Temperature monitoring thread
		//runTempThread();
	}
	return 0;
}
