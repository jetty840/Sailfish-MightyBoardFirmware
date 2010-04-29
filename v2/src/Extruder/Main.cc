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
#include "DebugPacketProcessor.hh"
#include "Host.hh"
#include "TemperatureThread.hh"
#include "Timeout.hh"
#include "ExtruderMotor.hh"
#include "ThermistorTable.hh"
#include <avr/interrupt.h>
#include "EepromMap.hh"
#include "ExtruderBoard.hh"
#include "MotorController.hh"

void runHostSlice();

void reset() {
	// Intialize various modules
	initThermistorTables();
	eeprom::init();
	ExtruderBoard::getBoard().reset();
	MotorController::getController().reset();
}

int main() {
	reset();
	ExtruderBoard::getBoard().indicateError(1);
	sei();
	while (1) {
		// Host interaction thread.
		runHostSlice();
		// Temperature monitoring thread
		runTempSlice();
		// Motor update thread
		MotorController::runMotorSlice();
	}
	return 0;
}

