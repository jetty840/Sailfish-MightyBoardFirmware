/*
 * T4-Commands.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "UART.hh"
#include "DebugPacketProcessor.hh"
#include "Host.hh"
#include "TemperatureThread.hh"
#include "Timeout.hh"
#include "DebugPin.hh"
#include "ExtruderMotor.hh"
#include "ThermistorTable.hh"
#include <avr/interrupt.h>
#include "EepromMap.hh"
#include "ExtruderBoard.hh"

void runHostSlice();

int main() {
	// Intialize various modules
	initThermistorTables();
	initEeprom();
	ExtruderBoard::getBoard().reset();
	sei();
	while (1) {
		// Host interaction thread.
		runHostSlice();
		// Temperature monitoring thread
		runTempSlice();
	}
	return 0;
}
