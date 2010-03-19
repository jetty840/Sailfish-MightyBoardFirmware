/*
 * T4-Commands.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "UART.hh"
#include "DebugPacketProcessor.hh"
#include "QueryPacketProcessor.hh"
#include "Host.hh"
#include "TemperatureThread.hh"
#include "Timeout.hh"
#include "DebugPin.hh"
#include "AnalogPin.hh"
#include "Timers.hh"
#include "HeatingElement.hh"
#include "ExtruderMotor.hh"
#include "ThermistorTable.hh"
#include <avr/interrupt.h>
#include "EepromMap.hh"
#include "ExtruderBoard.hh"

void runHostSlice();

int main() {
	// Intialize various modules
	UART& uart = ExtruderBoard::getBoard().getHostUART();
	uart.enable(true);
	uart.in_.reset();
	startTimers();
	initHeatingElement();
	initExtruderMotor();
	initThermistorTables();
	initEeprom();
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
