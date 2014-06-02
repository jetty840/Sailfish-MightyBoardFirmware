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
/// Note that Marlin uses 10C
#define PID_BYPASS_DELTA 15

/// Number of bad sensor readings we need to get in a row before shutting off the heater
const uint8_t SENSOR_MAX_BAD_READINGS = 15;

/// Number of temp readings to be at target value before triggering newTargetReached
/// with bad seating of thermocouples, we sometimes get innacurate reads
const uint16_t TARGET_CHECK_COUNT = 5;

/// If we read a temperature higher than this, shut down the heater
const int16_t HEATER_CUTOFF_TEMPERATURE = 300;

/// temperatures below setting by this amount will flag as "not heating up"
const int16_t HEAT_FAIL_THRESHOLD = 30;

// don't trigger heating up checking for target temperatures less than this
const int16_t HEAT_FAIL_CHECK_THRESHOLD = 30;

/// if the starting temperature is less than this amount, we will check heating progress
/// to get to this temperature, the heater has already been checked.
const int16_t HEAT_CHECKED_THRESHOLD = 50;

/// timeout for heating all the way up
const uint32_t HEAT_UP_TIME = 300000000;  //five minutes

/// timeout for showing heating progress
const uint32_t HEAT_PROGRESS_TIME = 90000000; // 90 seconds


/// threshold above starting temperature we check for heating progres
const int16_t HEAT_PROGRESS_THRESHOLD = 10;

Heater::Heater(TemperatureSensor& sensor_in,
               HeatingElement& element_in,
               micros_t sample_interval_micros_in,
               uint16_t eeprom_base_in, bool timingCheckOn, uint8_t calibration_offset) :
		sensor(sensor_in),
		element(element_in),
		sample_interval_micros(sample_interval_micros_in),
		eeprom_base(eeprom_base_in),
		heat_timing_check(timingCheckOn),
		calibration_eeprom_offset(calibration_offset)
{
	reset();
}

void Heater::reset() {
	// TODO: Reset sensor, element here?

	current_temperature = 0;
	startTemp = 0;
	paused_set_temperature = 0;

	// Deviation from MBI: at this point, MBI's reset() just repeats
	// the exact same sequence of statements as are found in abort().
	// So, let's save code space and just call abort().
	abort();
}

void Heater::abort() {
	fail_state = false;
	fail_count = 0;
	fail_mode = HEATER_FAIL_NONE;
	value_fail_count = 0;
	bypassing_PID = false;
	heatingUpTimer = Timeout();
	heatProgressTimer = Timeout();
	progressChecked = false;
	newTargetReached = false;
	is_paused = false;
	is_disabled = false;

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
	//next_sense_timeout.start(sample_interval_micros);

	// Deviation from MBI
	// Seems like a bad idea: what happens when there's a value already there which isn't 0x00 nor 0xff??
	// calibration_offset = eeprom::getEeprom8(eeprom_offsets::HEATER_CALIBRATION + calibration_eeprom_offset, 0);
	// calibration_offset = 0;
}

void Heater::disable(bool on) {
	if ( on )
		// This will ensure that everything is pristine and
		//   any setpoint has been cleared.  This then ensures
		//   that other tests such as isHeating(), isCooling(),
		//   has_reached_target_temperature() all return false.
		reset();
	is_disabled = on;
}

/*  Function logs the inital temp to the startTemp value,
  starts progress timers to avoid heatup failure, and sets the
  new target temperature for this heater.
  @param temp: temperature in degrees C. Zero degrees indicates
  'disable heaters'
 */
void Heater::set_target_temperature(int16_t target_temp)
{
       // clip our set temperature if we are over temp.
	int16_t maxtemp = (calibration_eeprom_offset == 2) ? MAX_HBP_TEMP : MAX_VALID_TEMP;
	if ( target_temp > maxtemp )
		target_temp = maxtemp;
	else if ( target_temp < 0 )
		target_temp = 0;

	// Presently, MBI's code is broken when a new temp is set for
	// a paused heater.  In MBI's fw, the paused heater's temp is
	// changed to the new temp and thus the heater does not act
	// like it is paused -- it starts drawing current if the new
	// temp is higher than the "holding" temp.  Furthermore, the
	// heater is left with the "paused" flag set and the paused
	// temp unchanged.  So, when the heater is unpaused, it reverts
	// to it's old target temp instead of the new target temp.
	// Because of this bug in the MBI fw, if you turn off a heater
	// which is paused by setting its temp to 0, that setting is
	// lost when the heater is unpaused -- the heater switches on
	// to a non-zero temp!

	// So, we check in Sailfish to see if the heater is paused.
	// If so, then we only drop the holding temp if the new temp
	// is lower than the holding temp.  And then we update the
	// paused_set_temp with the new target temp so that when the
	// heater is unpaused, it will go to the new temp.

	if ( is_paused) {
	     if (target_temp < pid.getTarget())
		  pid.setTarget(target_temp);
	     if (target_temp == 0)
		  is_paused = false;
	     paused_set_temperature = target_temp;
	     return;
	}

	if ( target_temp > 0 ) {
		BOARD_STATUS_CLEAR(Motherboard::STATUS_HEAT_INACTIVE_SHUTDOWN);
	}

	newTargetReached = false;

	if ( has_failed() || is_disabled ) {
		// 17 Dec 2012
		// MBI fw sets target to "temp" and not "0"
		// Seems like a bug in the MBI fw
		pid.setTarget(0);
		return;
	}

	if ( heat_timing_check ) {
		startTemp = current_temperature;
		progressChecked = false;
		value_fail_count = 0;

		// start a progress timer to verify we are getting temp change over time.
		if(target_temp > HEAT_FAIL_CHECK_THRESHOLD){
			// if the current temp is greater than a (low) threshold, don't check the heating up time, because
			// we've already done that to get to this temperature
			if((target_temp > (current_temperature + HEAT_PROGRESS_THRESHOLD)) && (current_temperature < HEAT_CHECKED_THRESHOLD))
			{	heatProgressTimer.start(HEAT_PROGRESS_TIME);}
			else
			{	heatProgressTimer = Timeout(); }

			heatingUpTimer.start(HEAT_UP_TIME);
		}
		else{
			heatingUpTimer = Timeout();
			heatProgressTimer = Timeout();
		}
	}
	pid.setTarget(target_temp);
}

// We now define target hysteresis, used as PID over/under range.
#define TARGET_HYSTERESIS 2

/// Returns true if the current PID temperature is within tolerance
/// of the expected current temperature.
bool Heater::has_reached_target_temperature()
{
	// MBI firmware does not check for a BAD_TEMPERATURE
	// that can lead to this routine returning TRUE when
	// BAD_TEMPERATURE is a large value such as 1024

	// Not clear offhand what this should return for a disabled heater.
	//
	// If we do not test is_disabled, then this call will return true since
	// for a disabled heater, newTargetReached == false, current_temperatute == 0,
	// and pid.getTarget() == 0.
	//
	// However if some code is errantly waiting for a disabled heater to
	// come to temp, then that is a bug that the code is inquiring about
	// a disabled heater.  So let's return false here and if it causes
	// a problem upstack, then fix the broken upstack logic.

	if ( is_paused || is_disabled || current_temperature >= BAD_TEMPERATURE )
		return false;

	// flag temperature reached so that PID variations don't trigger this
	// a second time
	if ( !newTargetReached ) {
		if ( (current_temperature >= (pid.getTarget() - TARGET_HYSTERESIS)) &&
		     (current_temperature <= (pid.getTarget() + TARGET_HYSTERESIS)) ) {
			//	reached_count++;
			//	if(reached_count >= TARGET_CHECK_COUNT){
			newTargetReached = true;
			//		}
		}
	}
	return newTargetReached;
}

bool Heater::isHeating(){
       return (pid.getTarget() > 0) && !has_reached_target_temperature() && !fail_state;
}

bool Heater::isCooling(){
	return (current_temperature > get_set_temperature()) && !has_reached_target_temperature() && !fail_state;
}

int16_t Heater::getDelta(){
	int target = pid.getTarget();
	int temp   = (int)(0.5 + sensor.getTemperature());
	return (int16_t)((target > temp) ? target - temp : temp - target);
}

void Heater::manage_temperature() {

        if ( is_disabled )
	    return;

	// if (next_sense_timeout.hasElapsed()) {
	//	next_sense_timeout.start(sample_interval_micros);
		switch (sensor.update()) {
		case TemperatureSensor::SS_ADC_BUSY:
		case TemperatureSensor::SS_ADC_WAITING:
			// We're waiting for the ADC, so don't update the temperature yet.
			return;
		case TemperatureSensor::SS_OK:
			// Result was ok, so reset the fail counter, and continue.
			fail_count = 0;
			break;
		case TemperatureSensor::SS_BAD_READ:
			// we got a read for the heater that is outside of the expected range
			fail_count++;

			if (fail_count > SENSOR_MAX_BAD_READINGS) {
				fail_mode = HEATER_FAIL_BAD_READS;
				fail();
			}
			return;
		case TemperatureSensor::SS_ERROR_UNPLUGGED:
		default:
			// If we get too many bad readings in a row, shut down the heater.
			fail_count++;

			if (fail_count > SENSOR_MAX_BAD_READINGS) {
				fail_mode = HEATER_FAIL_NOT_PLUGGED_IN;
				fail();
			}
			current_temperature = BAD_TEMPERATURE + 1;
			return;
		}

		float fp_current_temp = sensor.getTemperature(); // + calibration_offset;
		current_temperature = (int)(0.5 + fp_current_temp);

		if (!is_paused){
			uint8_t old_value_count = value_fail_count;
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
				if(heatProgressTimer.hasElapsed()){
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
			if(heatingUpTimer.hasElapsed() && has_reached_target_temperature() && (current_temperature < (pid.getTarget() - HEAT_FAIL_THRESHOLD))){
					value_fail_count++;

					if (value_fail_count > SENSOR_MAX_BAD_READINGS) {
						fail_mode = HEATER_FAIL_DROPPING_TEMP;
						fail();
						return;
					}
			}
			// if no bad heat reads have occured, clear the fail count
			// we don't want this to add up continually forever
			if(value_fail_count == old_value_count)
				value_fail_count = 0;
		}
	if (fail_state) {
		return;
	}

	next_pid_timeout.start(UPDATE_INTERVAL_MICROS);

	int delta = pid.getTarget() - current_temperature;

	if ( bypassing_PID && (delta < PID_BYPASS_DELTA) ) {
		bypassing_PID = false;
		pid.reset_state();
	}
	else if ( !bypassing_PID && (delta > PID_BYPASS_DELTA + 10) ) {
		bypassing_PID = true;
	}

	if ( bypassing_PID )
		set_output(255);
	else {
		int mv = 0;
		if ( pid.getTarget() != 0 ) {
			mv = pid.calculate(fp_current_temp);
			// offset value to compensate for heat bleed-off.
			// There are probably more elegant ways to do this,
			// but this works pretty well.
#if HEATER_OFFSET_ADJUSTMENT
			mv += HEATER_OFFSET_ADJUSTMENT;
#endif
			// clamp value
			if (mv < 0) mv = 0;
			else if (mv > 255) mv = 255;
		}
		set_output(mv);
	}
}


// wait on heating the heater until told to continue
// @param on set pause to on or off
void Heater::Pause(bool on){

	// don't pause / un-pause again
	if(is_disabled || is_paused == on)
		return;

	// don't pause if heater is not on
	if (on && !isHeating())
		return;

	if ( on ) {
		//set output to zero
		paused_set_temperature = get_set_temperature();
		// Don't accidentally set the temp to 1024!
		set_target_temperature(current_temperature < BAD_TEMPERATURE ? current_temperature : 0);
		// clear heatup timers
		heatingUpTimer = Timeout();
		heatProgressTimer = Timeout();
		// clear reached target temperature
		newTargetReached = false;
		is_paused = true; // do after get_set_temperature()
	}else{
		// restart heatup
		is_paused = false;
		set_target_temperature(paused_set_temperature);
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
	set_target_temperature(0);
	set_output(0);
	Motherboard::getBoard().heaterFail(fail_mode, calibration_eeprom_offset);
}
