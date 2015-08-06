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

#include "Compat.hh"
#include "Configuration.hh"
#include "ExtruderBoard.hh"
#include "HeatingElement.hh"
#include "CoolingFan.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/sfr_defs.h>
#include <avr/io.h>

#if defined(HAS_VIKI_INTERFACE)
#include "Motherboard.hh"
#endif

ExtruderBoard::ExtruderBoard(uint8_t slave_id_in, Pin HeaterPin_In, Pin FanPin_In,
			     THERMOCOUPLE_TYPE thermocouple_channel, uint16_t eeprom_base) :
     extruder_thermocouple(thermocouple_channel, FOO_ARG(THERMOCOUPLE_SCK), FOO_ARG(THERMOCOUPLE_SO)),
     extruder_element(slave_id_in),
     extruder_heater(extruder_thermocouple, extruder_element,
		     (eeprom_base + toolhead_eeprom_offsets::EXTRUDER_PID_BASE), true, slave_id_in),
     coolingFan(extruder_heater, (eeprom_base + toolhead_eeprom_offsets::COOLING_FAN_SETTINGS),
		FanPin_In),
     Heater_Pin(HeaterPin_In),
     eeprom_base((uint8_t*)eeprom_base),
     is_disabled(false)
#if defined(HAS_VIKI_INTERFACE)
     , active_heaters(0)
#endif
{
}

void ExtruderBoard::disable(bool state) {
     is_disabled = state;
     extruder_heater.disable(state);
}

void ExtruderBoard::reset() {
     // Set the output mode for the mosfets.
     Heater_Pin.setValue(false);
     Heater_Pin.setDirection(true);

     extruder_heater.reset();
     extruder_thermocouple.init();
     coolingFan.reset();

     is_disabled = false;
}

void ExtruderBoard::runExtruderSlice() {
     if ( is_disabled )
	  return;
     extruder_heater.manage_temperature();
     coolingFan.manageCoolingFan();
}

void ExtruderBoard::setFan(uint8_t on) {
     coolingFan.enable(on);
}

// Turn on/off PWM for Extruder Two
void pwmExB_On(bool on) {
     if ( on ) {
	  EXB_PWR_TCCRn |= EXB_PWR_TCCRn_on;
     }
     else {
	  EXB_PWR_TCCRn &= EXB_PWR_TCCRn_off;
     }
}

// Turn on/off PWM for Extruder One
void pwmExA_On(bool on) {
     if ( on ) {
	  EXA_PWR_TCCRn |= EXA_PWR_TCCRn_on;
     }
     else {
	  EXA_PWR_TCCRn &= EXA_PWR_TCCRn_off;
     }
}

ExtruderHeatingElement::ExtruderHeatingElement(uint8_t id) :
     heater_id(id)
{
}

void ExtruderHeatingElement::setHeatingElement(uint8_t value) {
     ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	  if ( heater_id == 0 ) {
	       if ( value == 0 || value == 255 ) {
		    // 100% on or 100% off
		    pwmExA_On(false);
		    EXA_PWR.setValue(value == 255);
	       }
	       else {
		    EXA_PWR_OCRn = value;
		    pwmExA_On(true);
	       }
	  }
	  else if ( heater_id == 1 ) {
	       if ( value == 0 || value == 255 ) {
		    // 100% on or 100% off
		    pwmExB_On(false);
		    EXB_PWR.setValue(value == 255);
	       }
	       else {
		    EXB_PWR_OCRn = value;
		    pwmExB_On(true);
	       }
	  }
     }
}
