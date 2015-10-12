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
#ifndef SIMULATOR
#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
#include <avr/pgmspace.h>
#endif
#endif

#define TEMP_OVERSAMPLE 8

#define ADC_THERMISTOR_DISCONNECTED(t) (t < (22 * TEMP_OVERSAMPLE))
#define ADC_THERMOCOUPLE_DISCONNECTED(t) (t > (1000 * TEMP_OVERSAMPLE))

// On 1 August 2015, 9 tables total would fit and not
// cause alignment errors with constructor function tables (ctor), etc.
// So, keeping things to 8 tables to keep a little margin

#define TABLE_THERMOCOUPLE_K  0
#define TABLE_HBP_THERMISTOR  1
#define TABLE_EXT_THERMISTOR  2
#define TABLE_3_THERMISTOR    3
#define TABLE_4_THERMISTOR    4
#define TABLE_5_THERMISTOR    5
#define TABLE_6_THERMISTOR    6
#define TABLE_7_THERMISTOR    7
#define TABLE_COUNT           8

#define THERM_INDEX_EPCOS     6

namespace TemperatureTable{

/// Translate a temperature reading into degrees Celcius, using the provided lookup table.
/// @param[in] reading Thermistor/Thermocouple voltage reading, in ADC counts
/// @param[in] table_idx therm_tables index of the temperature lookup table
/// @return Temperature reading, in degrees Celcius
float TempReadtoCelsius(int16_t reading, uint8_t table_idx, float max_allowed_value);

#ifndef SIMULATOR
#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
const prog_uchar *getThermistorName(uint8_t idx);
#endif
#endif
}

#endif // THERMISTOR_TABLE
