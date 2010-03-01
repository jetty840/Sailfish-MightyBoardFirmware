/*
 * T4-Commands.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "UART.hh"
#include "DebugPacketProcessor.hh"
#include "QueryPacketProcessor.hh"
#include "HostThread.hh"
#include "TemperatureThread.hh"
#include "Timeout.hh"
#include "DebugPin.hh"
#include "AnalogPin.hh"
#include "Timers.hh"
#include "HeatingElement.hh"
#include <avr/interrupt.h>

void runHostSlice();

int main() {
	// Intialize various modules
	uart[0].enable(true);
	uart[0].in_.reset();
	startTimers();
	initHeatingElement();
	initAnalogPins(_BV(3));
	sei();
	setDebugLED(true);
	while (1) {
		// Host interaction thread.
		runHostSlice();
		// Temperature monitoring thread
		runTempSlice();
	}
	return 0;
}
