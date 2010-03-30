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

#include "Timeout.hh"
#include "ExtruderBoard.hh"

Timeout::Timeout() : active(false), elapsed(false) {}

void Timeout::start(uint32_t duration_micros) {
	active = true;
	elapsed = false;
	end_stamp_micros = ExtruderBoard::getBoard().getCurrentMicros() + duration_micros;
}

bool Timeout::hasElapsed() {
	if (active && !elapsed) {
		if ((end_stamp_micros - ExtruderBoard::getBoard().getCurrentMicros()) <= 0) {
			active = false;
			elapsed = true;
		}
	}
	return elapsed;
}

void Timeout::abort() {
	active = false;
}
