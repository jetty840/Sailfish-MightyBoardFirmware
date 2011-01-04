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


#ifndef THERMOCOUPLE_HH_
#define THERMOCOUPLE_HH_

#include "TemperatureSensor.hh"
#include "AvrPort.hh"

class Thermocouple : public TemperatureSensor {
private:
	Pin cs_pin;
	Pin sck_pin;
	Pin so_pin;
public:
	Thermocouple(const Pin& cs,const Pin& sck,const Pin& so);
	void init();
	// True if update initiated, false otherwise
	SensorState update();
};
#endif // THERMOCOUPLE_HH_
