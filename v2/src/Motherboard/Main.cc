/*
 * T4-Commands.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "UART.hh"
#include "PSU.hh"
#include "DebugPacketProcessor.hh"
#include "HostThread.hh"
#include "QueryPacketProcessor.hh"
#include <avr/interrupt.h>
#include "Timeout.hh"
#include "DebugPin.hh"
#include "Steppers.hh"

void runToolSlice();
void runHostSlice();
void runSDSlice();
void runCommandSlice();

int main() {
	//
	initPsu();
	// Intialize various modules
	uart[0].enable(true);
	uart[0].in_.reset();
	uart[1].enable(true);
	sei();
	//steppers.setTarget(points[0],500);
	int point_idx = 1;
	setDebugLED(true);
	while (1) {
		// Toolhead interaction thread.
		//runToolSlice();
		// Host interaction thread.
		runHostSlice();
		// SD command buffer read/refill thread.
		//runSDSlice();
		// Command handling thread.
		runCommandSlice();
	}
	return 0;
}
