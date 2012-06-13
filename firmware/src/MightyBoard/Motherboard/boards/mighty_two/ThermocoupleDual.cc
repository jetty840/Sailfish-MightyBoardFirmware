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



#include "ThermocoupleDual.hh"
#include "Motherboard.hh"


DualThermocouple::DualThermocouple(uint8_t channel_in) :
	channel(channel_in),
	thermocouple_reader(Motherboard::getBoard().getThermocoupleReader())
{
}

void DualThermocouple::init() {
	
	current_temp = 0;
}


DualThermocouple::SensorState DualThermocouple::update() {

	bool bad_temperature = false; // Indicate a disconnected state
	
	uint16_t temperature_read = thermocouple_reader.GetChannelTemperature(channel);
	
	if (bad_temperature) {
	  // Set the temperature to 1024 as an error condition
	  current_temp = BAD_TEMPERATURE;
	  return SS_ERROR_UNPLUGGED;
	}

	current_temp = temperature_read;
	return SS_OK;
}
