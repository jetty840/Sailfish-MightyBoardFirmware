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

#include "Eeprom.hh"
#include "EepromMap.hh"
#include "Configuration.hh"
#include "SoftI2cManager.hh"
#include "DigiPots.hh"
#include "stdio.h"
#include "Timeout.hh"
#include "AnalogPin.hh"
#include "Motherboard.hh"

DigiPots::DigiPots(const Pin& pot,
                                   const uint16_t &eeprom_base_in, 
                                   const uint8_t vref_measure_pin_in) :
    
    pot_pin(pot),
    eeprom_base(eeprom_base_in),
    vref_measure_pin(vref_measure_pin_in) {
}

void DigiPots::init(const uint8_t idx) {
	
	eeprom_pot_offset = idx;
	CalibrateVref();
    resetPots();	
}

void DigiPots::resetPots()
{
    SoftI2cManager i2cPots = SoftI2cManager::getI2cManager();
    i2cPots.start(0b01011110 | I2C_WRITE, pot_pin);
    i2cPots.write(eeprom::getEeprom8(eeprom_base + eeprom_pot_offset, 0), pot_pin);
    i2cPots.stop();
}

void DigiPots::setPotValue(const uint8_t val)
{
	
	// we're going to want to calibrate each val with the appropriate vref adjust	
	//	set_value = GetCalibatedValue();
    SoftI2cManager i2cPots = SoftI2cManager::getI2cManager();
    i2cPots.start(0b01011110 | I2C_WRITE, pot_pin);
    i2cPots.write(val > digi_pot_max ? digi_pot_max : val, pot_pin);
    i2cPots.stop(); 
}

#define DIGI_POT_MAX_DEFAULT	118 ///assume max vref is 1.95V  (allowable vref for max current rating of stepper is 1.814)
#define ADC_READ_TIME			1000000 /// one second - temporary

void DigiPots::CalibrateVref() {

	if (vref_measure_pin == NULL){
		digi_pot_max = DIGI_POT_MAX_DEFAULT;
		return;
	}
	
	digi_pot_max = 128;
	
	/// setup analog pin
	initAnalogPin(vref_measure_pin);
	
	/// setPotValueHigh
	setPotValue(127);
	
	Timeout adc_read_timeout = Timeout();
	adc_read_timeout.start(ADC_READ_TIME);
	
	volatile int16_t raw_value = 0;
	volatile bool read_finished = false;
	
	/// read analog pin
	while (!startAnalogRead(vref_measure_pin,&raw_value, &read_finished) && !adc_read_timeout.hasElapsed()){;} 
	
	while(!read_finished && !adc_read_timeout.hasElapsed()){;}
	
	/// if adc has not returned a valid read within expected time, set
	/// digi pots to default calibration
	if(adc_read_timeout.hasElapsed()){
		/// error message
		digi_pot_max = DIGI_POT_MAX_DEFAULT;
	}
	
	uint16_t vref_high = raw_value;
	
	/// compute linear high that makes vref == 1.8
	
	/// store vref max value

}
