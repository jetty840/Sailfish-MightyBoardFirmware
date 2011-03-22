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

#include "DebugPacketProcessor.hh"
#include "Host.hh"
#include "Tool.hh"
#include "Command.hh"
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "Timeout.hh"
#include "Steppers.hh"
#include "Motherboard.hh"
#include "SDCard.hh"
#include "EepromMap.hh"

void reset(bool hard_reset) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		Motherboard& board = Motherboard::getBoard();
		sdcard::reset();
		steppers::abort();
		command::reset();
		eeprom::init();
		board.reset();
		sei();
		// If we've just come from a hard reset, wait for 2.5 seconds before
		// trying to ping an extruder.  This gives the extruder time to boot
		// before we send it a packet.
		if (hard_reset) {
			Timeout t;
			t.start(1000L*2500L); // wait for 2500 ms
			while (!t.hasElapsed());
		}
		if (!tool::reset())
		{
			// Fail, but let it go; toggling the PSU is dangerous.
		}
	}
}

int main() {
	Motherboard& board = Motherboard::getBoard();
	steppers::init(Motherboard::getBoard());
	reset(true);
	sei();
	while (1) {
		// Toolhead interaction thread.
		tool::runToolSlice();
		// Host interaction thread.
		runHostSlice();
		// Command handling thread.
		command::runCommandSlice();
		// Motherboard slice
		board.runMotherboardSlice();
	}
	return 0;
}
