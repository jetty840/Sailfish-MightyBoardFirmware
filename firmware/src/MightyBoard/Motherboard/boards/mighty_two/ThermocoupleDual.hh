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

#ifndef THERMOCOUPLE_DUAL_HH_
#define THERMOCOUPLE_DUAL_HH_

#include "TemperatureSensor.hh"
#include "ThermocoupleReader.hh"
#include "Pin.hh"

#define THERMOCOUPLE_UPDATE_RATE	250000 ///250ms - read 4 times per second (2 per channel)

/// The thermocouple module provides a bitbanging driver that can read the
/// temperature from (chip name) sensor, and also report on any error conditions.
/// \ingroup SoftwareLibraries
class DualThermocouple : public TemperatureSensor {
private:
	uint8_t channel;
	
public:
        /// Create a new thermocouple instance, and attach it to the given pins.
        /// \param [in] cs Chip Select (output).
        /// \param [in] sck Clock Pin (output). Can be shared with other thermocouples.
        /// \param [in] so Data Pin (input)
	DualThermocouple(uint8_t channel);

	void init();

	SensorState update();
};
#endif // THERMOCOUPLE_DUAL_HH_
