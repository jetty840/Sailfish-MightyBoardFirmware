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

#include "ExtruderBoard.hh"
#include "Configuration.hh"
#include "HeatingElement.hh"
#include "CoolingFan.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/sfr_defs.h>
#include <avr/io.h>


ExtruderBoard::ExtruderBoard(uint8_t slave_id_in, Pin HeaterPin_In, Pin FanPin_In,
		Pin ThermocouplePin_In,	uint16_t eeprom_base) :
     		extruder_thermocouple(ThermocouplePin_In,THERMOCOUPLE_SCK,THERMOCOUPLE_SO),
     		extruder_element(slave_id_in),
     		extruder_heater(extruder_thermocouple,extruder_element,SAMPLE_INTERVAL_MICROS_THERMOCOUPLE,
				(eeprom_base+ toolhead_eeprom_offsets::EXTRUDER_PID_BASE), true, slave_id_in),
      		coolingFan(extruder_heater, (eeprom_base + toolhead_eeprom_offsets::COOLING_FAN_SETTINGS), FanPin_In),
      		slave_id(slave_id_in),
      		Heater_Pin(HeaterPin_In),
		eeprom_base((uint8_t*)eeprom_base)
{
}

void ExtruderBoard::reset() {


	// Set the output mode for the mosfets.  
	Heater_Pin.setValue(false);
	Heater_Pin.setDirection(true);

	extruder_heater.reset();
	extruder_thermocouple.init();
	coolingFan.reset();

}

void ExtruderBoard::runExtruderSlice() {

  extruder_heater.manage_temperature();
  coolingFan.manageCoolingFan();

}

void ExtruderBoard::setFan(uint8_t on)
{
	if(on)
		coolingFan.enable();
	else
		coolingFan.disable();
}

// Turn on/off PWM for Extruder Two (OC1A)
void pwmEx2_On(bool on) {
	if (on) {
		TCCR1A |= 0b10000000;
	} else {
		TCCR1A &= 0b00111111;
	}
}

// Turn on/off PWM for Extruder One (OC4A)
void pwmEx1_On(bool on) {
	if (on) {
		TCCR4A |= 0b10000000;
	} else {
		TCCR4A &= 0b00111111;
	} 
}

ExtruderHeatingElement::ExtruderHeatingElement(uint8_t id):
	heater_id(id)
{
}

void ExtruderHeatingElement::setHeatingElement(uint8_t value) {
	
    
  	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	   if(heater_id == 0)
	   { 
     		if (value == 0 || value == 255) {
			pwmEx1_On(false);
			EX1_PWR.setValue(value == 255);
			} else {
				OCR4A = value;
				pwmEx1_On(true);
				
				
			}
		}
		else if(heater_id == 1)
	   { 
     		if (value == 0 || value == 255) {
			pwmEx2_On(false);
			EX2_PWR.setValue(value == 255);
			
			} else {
				OCR1A = value;
				pwmEx2_On(true);
			}
		}
	}
}




