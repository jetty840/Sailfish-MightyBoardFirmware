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

#define THERM_TABLE_SIZE 20
#define SAMPLE_COUNT 4

/// The thermistor module provides a driver to read the value of a thermistor connected
/// to an analog pin, and convert it to a corrected temperature in degress Celcius.
/// \ingroup SoftwareLibraries
class Thermistor : public TemperatureSensor {
private:
        uint8_t analog_pin;                 ///< index of analog pin
        volatile int16_t raw_value;         ///< raw storage for asynchronous analog read
        volatile bool raw_valid;            ///< flag to state if raw_value contains valid data
        // TODO: This should come from the ADC!
        const static int ADC_RANGE = 1024;  ///< Maximum ADC value
        const static int MAX_TEMP = 255;
        int16_t sample_buffer[SAMPLE_COUNT];///< Buffer for sampled temperature data
        uint8_t next_sample;                ///< Index pointing to where the next sample should go in the buffer.
        const uint8_t table_index;          ///< EEPROM offset where the thermistor conversion table is located.

public:
        /// Create a new thermistor, attacheced to the given analog input pin, and using
        /// the given index to load the temperature conversion table.
        /// @param analog_pin Analog pin that the thermistor is connected to (input)
        /// @param table_index EEPROM offset where the thermistor conversion table is located
	Thermistor(uint8_t analog_pin, uint8_t table_index);

	void init();

	SensorState update();
};

#endif //THERMISTOR_H
