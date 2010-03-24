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

#ifndef THERMISTOR_HH_
#define THERMISTOR_HH_

#include "TemperatureSensor.hh"
#include "CircularBuffer.hh"
#include "AvrPort.hh"

const static uint16_t THERM_TABLE_SIZE = 20;

struct ThermTableEntry {
	uint16_t adc_;
	int16_t celsius_;
} __attribute__ ((packed));

class Thermistor : public TemperatureSensor {
private:
	uint8_t analog_pin_; // index of analog pin
	volatile uint16_t raw_value_; // raw storage for asynchronous analog read
	const static int ADC_RANGE = 1024;
	const static uint8_t SAMPLE_COUNT = 10;
	uint16_t sample_buffer_[SAMPLE_COUNT];
	uint8_t next_sample_;
	const uint8_t table_index_;

public:
	Thermistor(uint8_t analog_pin, uint8_t table_index);
	void init();
	void update();
};

#endif //THERMISTOR_H
