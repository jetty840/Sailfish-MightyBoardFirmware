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

#include <stdint.h>

#define ADC_THERMISTOR_DISCONNECTED(t) (t < 22)
#define ADC_THERMOCOUPLE_DISCONNECTED(t) (t > 1000)

namespace TemperatureTable{

#define TABLE_THERMOCOUPLE_K 0
#define TABLE_HBP_THERMISTOR 1
#define TABLE_EXT_THERMISTOR 2

/// Translate a temperature reading into degrees Celcius, using the provided lookup table.
/// @param[in] reading Thermistor/Thermocouple voltage reading, in ADC counts
/// @param[in] table_idx therm_tables index of the temperature lookup table
/// @return Temperature reading, in degrees Celcius
float TempReadtoCelsius(int16_t reading, uint8_t table_idx, float max_allowed_value);

}

#endif // THERMISTOR_TABLE
