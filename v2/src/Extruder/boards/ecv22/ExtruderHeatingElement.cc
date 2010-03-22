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

#include "ExtruderHeatingElement.hh"
#include <avr/sfr_defs.h>
#include <avr/io.h>
#include "DebugPin.hh"

// TIMER2 is used to PWM mosfet channel B on OC2A.
void ExtruderHeatingElement::init() {
	DDRB |= _BV(3);
	TCCR2A = 0b10000011;
	TCCR2B = 0b00000110;
	OCR2A = 0;
	TIMSK2 = 0; // no interrupts needed.
}

void ExtruderHeatingElement::setHeatingElement(uint8_t value) {
	if (value == 0) { TCCR2A = 0b00000011; }
	else { TCCR2A = 0b10000011; }
	OCR2A = value;
}
