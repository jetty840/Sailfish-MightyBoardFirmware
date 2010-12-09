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

#include "Thermistor.hh"
#include "ThermistorTable.hh"
#include "AnalogPin.hh"
#include <avr/eeprom.h>
#include <util/atomic.h>

Thermistor::Thermistor(uint8_t analog_pin_in, uint8_t table_index_in) :
analog_pin(analog_pin_in), next_sample(0), table_index(table_index_in) {
	for (int i = 0; i < SAMPLE_COUNT; i++) { sample_buffer[i] = 0; }
}

void Thermistor::init() {
	initAnalogPins(_BV(analog_pin));
}

bool Thermistor::update() {
	int16_t temp;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		temp = raw_value;
	}
	// initiate next read
	if (!startAnalogRead(analog_pin,&raw_value)) return false;

	sample_buffer[next_sample] = temp;
	next_sample = (next_sample+1) % SAMPLE_COUNT;

	// average
	int16_t cumulative = 0;
	for (int i = 0; i < SAMPLE_COUNT; i++) {
		cumulative += sample_buffer[i];
	}
	int16_t avg = cumulative / SAMPLE_COUNT;

	// TODO: this probably doesn't work for the (non thermistor) analog input pins.
	// If the calculated temperature is very high, then the thermistor is
	// likely disconnected. Report this as a failure.

	if (temp > ADC_RANGE - 4) {
		return false;
	}

	// TODO: Why don't we use averaging here?
	//current_temp = thermistorToCelsius(avg,table_index);
	current_temp = thermistorToCelsius(temp,table_index);

	return true;
}
