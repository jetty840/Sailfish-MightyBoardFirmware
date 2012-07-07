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

#ifndef TEMPERATURE_HH_
#define TEMPERATURE_HH_

#include <stdint.h>

/// Flag specifying that the temperature reading is invalid.
#define BAD_TEMPERATURE 1024

/// The temperature sensor interface is a standard interface used to communicate with
/// things that can sense temperatures.
/// \ingroup SoftwareLibraries
class TemperatureSensor {
protected:
        /// The last temperature reading from the sensor, in degrees Celcius, or
        /// #BAD_TEMPERATURE if the last reading is invalid.
	volatile int16_t current_temp;
public:
	enum SensorState {
		SS_OK,              ///< Temperature measured correctly
		SS_ADC_BUSY,        ///< Temperature failed to update because ADC is busy
		SS_ADC_WAITING,     ///< Temperature failed to update, still waiting for ADC
		SS_ERROR_UNPLUGGED, ///< Temperature failed to update, the sensor is unplugged.
		SS_BAD_READ			///< Temperature is outside the expected range
	};


	/// Get the last read temperature from the sensor. Note that you need to call
	/// update() at least once for this to return good data.
	/// \return The current temperature, in degrees Celcius, or #BAD_TEMPERATURE if the
	///         last read failed.
	int16_t getTemperature() const { return current_temp; }

	/// Initialize the temperature sensor hardware. Must be called before the temperature
	/// sensor can be used.
	virtual void init() {}

	/// Attempt to update the temperature sensor measurement.
	/// \return #SS_OK if the reading was successful, or an error.
	virtual SensorState update() =0;
};

#endif // TEMPERATURE_HH_
