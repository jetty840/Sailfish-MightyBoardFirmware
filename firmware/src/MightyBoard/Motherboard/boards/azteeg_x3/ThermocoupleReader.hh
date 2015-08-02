/*
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

#ifndef THERMOCOUPLE_READER_HH_
#define THERMOCOUPLE_READER_HH_

#include "Pin.hh"
#include "TemperatureSensor.hh"

#define TEMP_NSENSORS		3

#define THERM_READY		0
#define THERM_NOT_READY		1
#define THERM_ADC_BUSY		2

#define THERM_CHANNEL_ONE	0
#define THERM_CHANNEL_TWO	1
#define THERM_CHANNEL_HBP	2

class ThermocoupleReader {

public:

private:
     TemperatureSensor::SensorState error_code;

     volatile bool    finished;
     volatile int16_t raw;
     int16_t          accum;
     uint8_t          cnt, last, pindex, pin[TEMP_NSENSORS], sensor_types;
	 uint8_t          table_indices[TEMP_NSENSORS];
     float            temp[TEMP_NSENSORS];
     void reset();

public:
     ThermocoupleReader();
     void init();
     void initConfig();
     uint8_t update();
     uint8_t getLastUpdated() { return last; }
     TemperatureSensor::SensorState GetChannelTemperature(
	 uint8_t channel,
	 volatile float &read_temperature);

     /// safety value if ADC read is out of range
     const static int16_t MAX_TEMP = 400;
};

#endif // THERMOCOUPLE_READER_HH_
