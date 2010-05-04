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
#include "ExtruderBoard.hh"
#include "EepromMap.hh"

#define DEFAULT_P 6.0
#define DEFAULT_I 0.325
#define DEFAULT_D 36.0

// Offset to compensate for range clipping and bleed-off
#define HEATER_OFFSET_ADJUSTMENT 0

Heater::Heater(TemperatureSensor& sensor_in, HeatingElement& element_in) :
		sensor(sensor_in),
		element(element_in)
{
	reset();
}

void Heater::reset() {
	current_temperature = 0;

	float p = eeprom::getEepromFixed16(eeprom::EXTRUDER_PID_P_TERM,DEFAULT_P);
	float i = eeprom::getEepromFixed16(eeprom::EXTRUDER_PID_I_TERM,DEFAULT_I);
	float d = eeprom::getEepromFixed16(eeprom::EXTRUDER_PID_D_TERM,DEFAULT_D);

	pid.reset();
	if (p == 0 && i == 0 && d == 0) {
		p = DEFAULT_P; i = DEFAULT_I; d = DEFAULT_D;
	}
	pid.setPGain(p);
	pid.setIGain(i);
	pid.setDGain(d);
	pid.setTarget(0);
	next_pid_timeout.start(UPDATE_INTERVAL_MICROS);
	next_sense_timeout.start(SAMPLE_INTERVAL_MICROS);
}

void Heater::set_target_temperature(int temp)
{
	pid.setTarget(temp);
}

// We now define target hysteresis in absolute degrees.  The original
// implementation (+/-5%) was giving us swings of 10% in either direction
// *before* any artifacts of process instability came in.
#define TARGET_HYSTERESIS 2

bool Heater::hasReachedTargetTemperature()
{
	return (current_temperature >= (pid.getTarget() - TARGET_HYSTERESIS)) &&
			(current_temperature <= (pid.getTarget() + TARGET_HYSTERESIS));
}

int Heater::get_set_temperature() {
	return pid.getTarget();
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
	if (next_sense_timeout.hasElapsed()) {
		if (!sensor.update()) return;
		next_sense_timeout.start(SAMPLE_INTERVAL_MICROS);
	}
	if (next_pid_timeout.hasElapsed()) {
		next_pid_timeout.start(UPDATE_INTERVAL_MICROS);
		// update the temperature reading.
		current_temperature = get_current_temperature();

		int mv = pid.calculate(current_temperature);
		// offset value to compensate for heat bleed-off.
		// There are probably more elegant ways to do this,
		// but this works pretty well.
		mv += HEATER_OFFSET_ADJUSTMENT;
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
