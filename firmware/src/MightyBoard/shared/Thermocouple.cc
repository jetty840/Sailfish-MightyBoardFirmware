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


// We'll throw in nops to get the timing right (if necessary)
inline void nop() {
        asm volatile("nop"::);
        asm volatile("nop"::);
        asm volatile("nop"::);
        asm volatile("nop"::);
        asm volatile("nop"::);
}


Thermocouple::Thermocouple(const Pin& cs,const Pin& sck,const Pin& so) :
        cs_pin(cs),
        sck_pin(sck),
        so_pin(so)
{
}

void Thermocouple::init() {
	cs_pin.setDirection(true);
	sck_pin.setDirection(true);
	so_pin.setDirection(false);
	
	current_temp = 0;

//	cs_pin.setValue(true);   // Clock select is active low
//	sck_pin.setValue(false); // TODO: Is this a good idea?
}


Thermocouple::SensorState Thermocouple::update() {
	// TODO: Check timing against datasheet.
	cs_pin.setValue(false);
	nop();
	sck_pin.setValue(false);
	nop();

	int raw = 0;
	bool bad_temperature = false; // Indicate a disconnected state
	for (int i = 0; i < 16; i++) {
		sck_pin.setValue(true);
		nop();
		if (i >= 1 && i < 11) { // data bit... skip LSBs
			raw = raw << 1;
			if (so_pin.getValue()) { raw = raw | 0x01; }
		}
		if (i == 13) { // Safety check: Check for open thermocouple input
			if (so_pin.getValue()) {
			  bad_temperature = true;
			}
		}
		sck_pin.setValue(false);
		nop();
	}

	cs_pin.setValue(true);
	nop();
	sck_pin.setValue(false);

	if (bad_temperature) {
	  // Set the temperature to 1024 as an error condition
	  current_temp = BAD_TEMPERATURE;
	  return SS_ERROR_UNPLUGGED;
	}

	current_temp = raw;
	return SS_OK;
}
