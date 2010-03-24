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

#include "DebugPin.hh"
#include <avr/io.h>

#define DEBUG_PIN 5

void setDebugLED(bool on) {
	DDRB |= _BV(DEBUG_PIN);
	if (on) {
		PORTB |= _BV(DEBUG_PIN);
	} else {
		PORTB &= ~_BV(DEBUG_PIN);
	}
}
