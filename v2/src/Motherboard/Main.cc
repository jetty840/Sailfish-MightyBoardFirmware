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

void reset() {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		sdcard::reset();
		steppers::abort();
		command::reset();
		if (!tool::reset()) {
			// The tool didn't acknowledge our reset!  Force it off by toggling the PSU.
			Motherboard::getBoard().getPSU().turnOn(false);
			Timeout t;
			t.start(1000*300); // turn off for 300 ms
			while (!t.hasElapsed());
			Motherboard::getBoard().getPSU().turnOn(false);
		}
		Motherboard::getBoard().reset();
	}
}

int main() {
	steppers::init(Motherboard::getBoard());
	reset();
	sei();
	while (1) {
		// Toolhead interaction thread.
		tool::runToolSlice();
		// Host interaction thread.
		runHostSlice();
		// Command handling thread.
		command::runCommandSlice();
	}
	return 0;
}
