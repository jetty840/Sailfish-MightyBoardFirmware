/*
 * Copyright 2010 by Adam Mayer <adam@makerbot.com>
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



#include "Thermocouple.hh"
#include <avr/eeprom.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "ExtruderBoard.hh"

Thermocouple::Thermocouple(const Pin& cs,const Pin& sck,const Pin& so) :
	cs_pin(cs), sck_pin(sck), so_pin(so)
{
}

void Thermocouple::init() {
	cs_pin.setDirection(true);
	sck_pin.setDirection(true);
	so_pin.setDirection(false);
}

// We'll throw in nops to get the timing right (if necessary)
inline void nop() {
	asm volatile("nop"::);
	asm volatile("nop"::);
	asm volatile("nop"::);
	asm volatile("nop"::);
	asm volatile("nop"::);
}

bool Thermocouple::update() {
	cs_pin.setValue(false);
	nop();
	sck_pin.setValue(false);
	nop();
	int raw = 0;
	for (int i = 0; i < 16; i++) {
		sck_pin.setValue(true);
		nop();
		if (i >= 1 && i < 11) { // data bit... skip LSBs
			raw = raw << 1;
			if (so_pin.getValue()) { raw = raw | 0x01; }
		}
		if (i == 13) { // Safety check: Check for open thermocouple input
			if (so_pin.getValue()) {
				return false;
			}
		}
		sck_pin.setValue(false);
		nop();
	}
	cs_pin.setValue(true);
	nop();
	sck_pin.setValue(false);

	// Safety check: If we read 0, the thermocouple is probably backwards,
	// so return an error.
	if (raw == 0) {
		return false;
	}

	current_temp = raw;
	return true;
}
