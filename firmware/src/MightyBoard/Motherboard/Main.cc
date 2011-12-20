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

#include "Main.hh"
#include "DebugPacketProcessor.hh"
#include "Host.hh"
//#include "Tool.hh"
#include "Command.hh"
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "Timeout.hh"
#include "Steppers.hh"
#include "Motherboard.hh"
#include "SDCard.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"
//#include "ExtruderMotor.hh"
#include "ThermistorTable.hh"
//#include "ExtruderBoard.hh"
//#include "MotorController.hh"


void reset(bool hard_reset) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		
		uint8_t resetFlags = MCUSR & 0x0f;
		// present in extruder board main : generates a power-on-reset
		// may need specific tool behaviors to emulate this if hard_reset
		// MCUSR = 0x0;
	
		Motherboard& board = Motherboard::getBoard();
		sdcard::reset();
		steppers::abort();
		command::reset();
		eeprom::init();
		initThermistorTables();
		board.reset();	
	}
}

int main() {

	Motherboard& board = Motherboard::getBoard();
	steppers::init(Motherboard::getBoard());
	reset(true);
	sei();
	while (1) {
		// Host interaction thread.
		host::runHostSlice();
		// Command handling thread.
		command::runCommandSlice();
		// Motherboard slice
		board.runMotherboardSlice();
		
	}
	return 0;
}
