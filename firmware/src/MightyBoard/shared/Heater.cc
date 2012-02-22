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
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "Motherboard.hh"


/// Offset to compensate for range clipping and bleed-off
#define HEATER_OFFSET_ADJUSTMENT 0

/// PID bypass: If the set point is more than this many degrees over the
///             current temperature, bypass the PID loop altogether.
#define PID_BYPASS_DELTA 15

/// Number of bad sensor readings we need to get in a row before shutting off the heater
const uint8_t SENSOR_MAX_BAD_READINGS = 5;

/// If we read a temperature higher than this, shut down the heater
const uint16_t HEATER_CUTOFF_TEMPERATURE = 300;


/// temperatures below setting by this amount will flag as "not heating up"
const uint16_t HEAT_FAIL_THRESHOLD = 30;

/// timeout for heating all the way up
const uint32_t HEAT_UP_TIME = 300000000;  //five minutes

/// timeout for showing heating progress
const uint32_t HEAT_PROGRESS_TIME = 40000000; // 40 seconds


/// threshold above starting temperature we check for heating progres
const uint16_t HEAT_PROGRESS_THRESHOLD = 10;

Heater::Heater(TemperatureSensor& sensor_in,
               HeatingElement& element_in,
               micros_t sample_interval_micros_in,
               uint16_t eeprom_base_in, bool timingCheckOn) :
		sensor(sensor_in),
		element(element_in),
		sample_interval_micros(sample_interval_micros_in),
		eeprom_base(eeprom_base_in),
		heat_timing_check(timingCheckOn)
{
	reset();
}

void Heater::reset() {
	// TODO: Reset sensor, element here?

	current_temperature = 0;
	startTemp = 0;

	fail_state = false;
	fail_count = 0;
	fail_mode = HEATER_FAIL_NONE;
	value_fail_count = 0;

	heatingUpTimer = Timeout();
	heatProgressTimer = Timeout();
	progressChecked = false;
	newTargetReached = false;

	float p = eeprom::getEepromFixed16(eeprom_base+pid_eeprom_offsets::P_TERM_OFFSET,DEFAULT_P);
	float i = eeprom::getEepromFixed16(eeprom_base+pid_eeprom_offsets::I_TERM_OFFSET,DEFAULT_I);
	float d = eeprom::getEepromFixed16(eeprom_base+pid_eeprom_offsets::D_TERM_OFFSET,DEFAULT_D);

	pid.reset();
	if (p == 0 && i == 0 && d == 0) {
		p = DEFAULT_P; i = DEFAULT_I; d = DEFAULT_D;
	}
	pid.setPGain(p);
	pid.setIGain(i);
	pid.setDGain(d);
	pid.setTarget(0);
	next_pid_timeout.start(UPDATE_INTERVAL_MICROS);
	next_sense_timeout.start(sample_interval_micros);

}

/*  Function logs the inital temp to the startTemp value,
  starts progress timers to avoid heatup failure, and sets the
  new target temperature for this heater.
  @param temp: temperature in degrees C. Zero degrees indicates
  'disable heaters'
 */
#define MAX_VALID_TEMP 260

void Heater::set_target_temperature(int temp)
{
	// clip our set temperature if we are over temp.
	if(temp > MAX_VALID_TEMP) {
		temp = MAX_VALID_TEMP;
	}
	
	newTargetReached = false;
	
	if(heat_timing_check){
		startTemp = current_temperature;	
		progressChecked = false;
		value_fail_count = 0;
	
		// start a progress timer to verify we are getting temp change over time.
		if(temp > HEAT_FAIL_THRESHOLD){
			if(temp > startTemp + HEAT_PROGRESS_THRESHOLD)
				heatProgressTimer.start(HEAT_PROGRESS_TIME);
			else
				heatProgressTimer = Timeout();
				
			heatingUpTimer.start(HEAT_UP_TIME);
		}
		else{
			heatingUpTimer = Timeout();
			heatProgressTimer = Timeout();
		}
	}
	pid.setTarget(temp);
}

// We now define target hysteresis, used as PID over/under range.
#define TARGET_HYSTERESIS 2

/// Returns true if the current PID temperature is within tolerance
/// of the expected current temperature.
bool Heater::has_reached_target_temperature()
{
    // flag temperature reached so that PID variations don't trigger this
    // a second time
	if(!newTargetReached){
		if((current_temperature >= (pid.getTarget() - TARGET_HYSTERESIS)) &&
			(current_temperature <= (pid.getTarget() + TARGET_HYSTERESIS)))
		{	newTargetReached = true;}
	}
	return newTargetReached; 
}

int Heater::get_set_temperature() {
	return pid.getTarget();
}

int Heater::get_current_temperature()
{
	return current_temperature;
}

int Heater::getPIDErrorTerm() {
	return pid.getErrorTerm();
}

int Heater::getPIDDeltaTerm() {
	return pid.getDeltaTerm();
}

int Heater::getPIDLastOutput() {
	return pid.getLastOutput();
}

bool Heater::isHeating(){
       return (pid.getTarget() > 0) && !has_reached_target_temperature() && !fail_state;
}

int16_t Heater::getDelta(){
	
		uint16_t target = pid.getTarget();
		uint16_t temp = sensor.getTemperature();
		int16_t delta = (target > temp) ? target - temp : temp - target;
        return delta;
}


void Heater::manage_temperature() {
	

	if (next_sense_timeout.hasElapsed()) {
		
		next_sense_timeout.start(sample_interval_micros);
		switch (sensor.update()) {
		case TemperatureSensor::SS_ADC_BUSY:
		case TemperatureSensor::SS_ADC_WAITING:
			// We're waiting for the ADC, so don't update the temperature yet.
			current_temperature = 2;
			return;
			break;
		case TemperatureSensor::SS_OK:
			// Result was ok, so reset the fail counter, and continue.
			fail_count = 0;
			break;
		case TemperatureSensor::SS_ERROR_UNPLUGGED:
		default:
			// If we get too many bad readings in a row, shut down the heater.
			fail_count++;

			if (fail_count > SENSOR_MAX_BAD_READINGS) {
				fail_mode = HEATER_FAIL_NOT_PLUGGED_IN;
				fail();
			}
			current_temperature = 3;
			return;
			break;
		}

		current_temperature = sensor.getTemperature();
		// check that the the heater isn't reading above the maximum allowable temp
		if (current_temperature > HEATER_CUTOFF_TEMPERATURE) {
			value_fail_count++;

			if (value_fail_count > SENSOR_MAX_BAD_READINGS) {
				fail_mode = HEATER_FAIL_SOFTWARE_CUTOFF;
				fail();
				return;
			}
		}
		// check that the heater is heating up after target is set
		if(!progressChecked){
			if(heatProgressTimer.hasElapsed() && !progressChecked){ 
				if(current_temperature < (startTemp + HEAT_PROGRESS_THRESHOLD )){
					value_fail_count++;

					if (value_fail_count > SENSOR_MAX_BAD_READINGS) {
						fail_mode = HEATER_FAIL_NOT_HEATING;
						fail();
						return;
					}
				}
				else
					progressChecked = true;
			}
		}
		// check that the heater temperature does not drop when still set to high temp
		if(heatingUpTimer.hasElapsed() && (current_temperature < (pid.getTarget() - HEAT_FAIL_THRESHOLD))){
				value_fail_count++;

				if (value_fail_count > SENSOR_MAX_BAD_READINGS) {
					fail_mode = HEATER_FAIL_DROPPING_TEMP;
					fail();
					return;
				}
		}
	}
	if (fail_state) {
		return;
	}
	if (next_pid_timeout.hasElapsed()) {
		next_pid_timeout.start(UPDATE_INTERVAL_MICROS);

		int delta = pid.getTarget() - current_temperature;

		if( bypassing_PID && (delta < PID_BYPASS_DELTA) ) {
			bypassing_PID = false;

			pid.reset_state();
		}
		else if ( !bypassing_PID && (delta > PID_BYPASS_DELTA + 10) ) {
			bypassing_PID = true;
		}

		if( bypassing_PID ) {
			set_output(255);
		}
		else {
			int mv = pid.calculate(current_temperature);
			// offset value to compensate for heat bleed-off.
			// There are probably more elegant ways to do this,
			// but this works pretty well.
			mv += HEATER_OFFSET_ADJUSTMENT;
			// clamp value
			if (mv < 0) { mv = 0; }
			if (mv >255) { mv = 255; }
			if (pid.getTarget() == 0) { mv = 0; }
			set_output(mv);
				
		}
	}
}

void Heater::set_output(uint8_t value)
{
	element.setHeatingElement(value);
}

// mark as failed and report to motherboard for user messaging
void Heater::fail()
{
	fail_state = true;
	set_output(0);
	Motherboard::getBoard().heaterFail(fail_mode);
}

bool Heater::has_failed()
{
	return fail_state;
}
