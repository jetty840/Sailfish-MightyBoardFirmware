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

#include "Configuration.hh"
#include "Heater.hh"
#include "HeatingElement.hh"
#include "Thermistor.hh"
#include "DebugPin.hh"
#include "ExtruderBoard.hh"

Heater::Heater(TemperatureSensor& sensor_in, HeatingElement& element_in) :
		sensor(sensor_in),
		element(element_in),
		current_temperature(0),
		last_update(0)
{
	pid.reset();
	pid.setPGain(5.0);
	pid.setIGain(0.1);
	pid.setDGain(5.0);
	pid.setTarget(0);
	next_read_timeout.start(UPDATE_INTERVAL_MICROS);
}

void Heater::set_target_temperature(int temp)
{
	pid.setTarget(temp);
}

bool Heater::hasReachedTargetTemperature()
{
	return (current_temperature > (pid.getTarget() * 0.95)) &&
			(current_temperature < (pid.getTarget() * 1.1));
}

/**
 *  Samples the temperature and converts it to degrees celsius.
 *  Returns degrees celsius.
 */
int Heater::get_current_temperature()
{
	return sensor.getTemperature();
}


/*!
 Manages motor and heater based on measured temperature:
 o If temp is too low, don't start the motor
 o Adjust the heater power to keep the temperature at the target
 */
void Heater::manage_temperature()
{
	if (next_read_timeout.hasElapsed()) {
		if (!sensor.update()) return;
		next_read_timeout.start(UPDATE_INTERVAL_MICROS);
		// update the temperature reading.
		current_temperature = get_current_temperature();

		int mv = pid.calculate(current_temperature);
		// clamp value
		if (mv < 0) { mv = 0; }
		if (mv >255) { mv = 255; }
		set_output(mv);
	}
}

void Heater::set_output(uint8_t value)
{
	element.setHeatingElement(value);
}
