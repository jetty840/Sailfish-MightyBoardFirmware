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

#ifndef THERMISTOR_TABLE
#define THERMISTOR_TABLE

#define NUMTEMPS 20

#include <stdint.h>

namespace TemperatureTable{
	
enum therm_tables {
	table_thermistor = 0,
	table_thermocouple = 1,
	table_coldjunction = 2
};

/// Translate a temperature reading into degrees Celcius, using the provided lookup table.
/// @param[in] reading Thermistor/Thermocouple voltage reading, in ADC counts
/// @param[in] table_idx therm_tables index of the temperature lookup table
/// @return Temperature reading, in degrees Celcius
float TempReadtoCelsius(int16_t reading, int8_t table_idx, float max_allowed_value);

}

typedef struct {
	int16_t adc;
	int16_t value;
} Entry;

typedef Entry TempTable[NUMTEMPS];

extern const TempTable default_therm_table;

#endif // THERMISTOR_TABLE
