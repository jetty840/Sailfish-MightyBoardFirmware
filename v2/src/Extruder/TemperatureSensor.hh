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

class TemperatureSensor {
protected:
	// Maintained by the sensor update routine
	volatile uint16_t current_temp_;
public:
	// Return current estimated temperature in degrees Celsius.
	int16_t getTemperature() const { return current_temp_; }
	// Run this sensor's update routine
	virtual void update() =0;
};

#endif // TEMPERATURE_HH_
