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
#include "DebugPin.hh"
#include "AnalogPin.hh"
#include <avr/eeprom.h>
#include <util/atomic.h>

Thermistor::Thermistor(uint8_t analog_pin, uint8_t table_index) :
analog_pin_(analog_pin), next_sample_(0), table_index_(table_index) {
	for (int i = 0; i < SAMPLE_COUNT; i++) { sample_buffer_[i] = 0; }
}

void Thermistor::init() {
	initAnalogPins(_BV(analog_pin_));
}

void Thermistor::update() {
	uint16_t temp;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		temp = raw_value_;
	}
	// initiate next read
	startAnalogRead(analog_pin_,&raw_value_);

	sample_buffer_[next_sample_] = temp;
	next_sample_ = (next_sample_+1) % SAMPLE_COUNT;

	// average
	int16_t cumulative = 0;
	for (int i = 0; i < SAMPLE_COUNT; i++) {
		cumulative += sample_buffer_[i];
	}
	int16_t avg = cumulative / SAMPLE_COUNT;

	current_temp_ = thermistorToCelsius(avg,table_index_);
}
