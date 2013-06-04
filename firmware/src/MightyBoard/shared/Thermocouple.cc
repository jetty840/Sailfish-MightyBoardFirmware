/*
 * Copyright 2010 by Adam Mayer <adam@makerbot.com>
 *
 * Rewritten 2013 by Dan Newman <dan.newman@mtbaldy.us>
 *   MAX6675 init & reading code updated for clarity/simplicity,
 *     use fewer clock cycles, to return a floating point value,
 *     and to follow the spec sheet more closely.
 *   MAX3855 code added.
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
	cs_pin.setValue(true);   // Clock select is active low
	current_temp = 0;
}

#ifndef MAX31855

Thermocouple::SensorState Thermocouple::update() {
	cs_pin.setValue(false);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
	nop();
#pragma GCC diagnostic pop

	uint16_t raw  = 0;
	for (int8_t i = 15; i >= 0; i--)
	{
		sck_pin.setValue(false);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
		nop();
#pragma GCC diagnostic ignored "-Winline"

		if ( so_pin.getValue() )
			raw |= (1 << i);

		sck_pin.setValue(true);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
		nop();
#pragma GCC diagnostic ignored "-Winline"
	}

	cs_pin.setValue(true);

	if ( raw & 0x04 ) {
		// Set the temperature to 1024 as an error condition
		current_temp = BAD_TEMPERATURE + 1;
		return SS_ERROR_UNPLUGGED;
	}

	current_temp = (raw >> 3) * 0.25;

	return SS_OK;
}

#else

Thermocouple::SensorState Thermocouple::update() {

	// TODO: Check timing against datasheet.
#pragma GCC diagnostic push
	sck_pin.setValue(false);
#pragma GCC diagnostic ignored "-Winline"
	nop();
	cs_pin.setValue(false);
#pragma GCC diagnostic ignored "-Winline"
	nop();
#pragma GCC diagnostic pop

	uint32_t raw = 0;
	for (uint8_t i = 0; i < 32; i++) {
		sck_pin.setValue(false);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
		nop();
#pragma GCC diagnostic pop
		raw <<= 1;
		if ( so_pin.getValue() ) 
			raw |= 1;
		sck_pin.setValue(true);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
		nop();
#pragma GCC diagnostic pop
	}

	cs_pin.setValue(true);

	// Evaluate the results

	if ( raw & 0x07 ) {
		// Set the temperature to 1024 as an error condition
		current_temp = BAD_TEMPERATURE + 1;
		return SS_ERROR_UNPLUGGED;
	}

	// Ignore the chip's internal temperature and status/error bits
	raw >>= 18;

	// Use the bottom 13 bits
	int16_t temp = (int16_t)(raw & 0x3fff);

	// Sign bit
	if ( raw & 0x2000 ) 
		temp |= 0xc000;

	current_temp = temp * 0.25;

	return SS_OK;
}

#endif
