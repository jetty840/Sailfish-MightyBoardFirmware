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
#include "Timeout.hh"
#include "ExtruderMotor.hh"
#include "ThermistorTable.hh"
#include <avr/interrupt.h>
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "ExtruderBoard.hh"
#include "MotorController.hh"

void reset() {
	cli();

	uint8_t resetFlags = MCUSR & 0x0f;
	MCUSR = 0x0;

	// Intialize various modules
	initThermistorTables();
	eeprom::init();
	ExtruderBoard::getBoard().reset(resetFlags);
	sei();
}

int main() {
	reset();
	while (1) {
		// Host interaction thread.
		runHostSlice();

		// Temperature monitoring thread
                ExtruderBoard::getBoard().runExtruderSlice();
	}
	return 0;
}

