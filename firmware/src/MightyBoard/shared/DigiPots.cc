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
#include "StepperAxis.hh"

DigiPots::DigiPots(const Pin& pot,
                                   const uint16_t &eeprom_base_in) :
    
    pot_pin(pot),
    eeprom_base(eeprom_base_in) {
}

void DigiPots::init(const uint8_t idx) {
	
	eeprom_pot_offset = idx;
    resetPots();
 
}

void DigiPots::resetPots()
{
    potValue = eeprom::getEeprom8(eeprom_base + eeprom_pot_offset, 0);
    setPotValue(potValue);
}

void DigiPots::setPotValue(const uint8_t val)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
    SoftI2cManager i2cPots = SoftI2cManager::getI2cManager();
#pragma GCC diagnostic pop

#ifdef DIGI_POT_WRITE_VERIFICATION
    uint8_t i = 0, actualDigiPotValue;
    do
    {
#endif
	i2cPots.start(0b01011110 | I2C_WRITE, pot_pin);
	if ( eeprom_pot_offset == Z_AXIS ) potValue = val > DIGI_POT_MAX_Z ? DIGI_POT_MAX_Z : val;
	else			    	   potValue = val > DIGI_POT_MAX_XYAB ? DIGI_POT_MAX_XYAB : val;
	i2cPots.write(potValue, pot_pin);
	i2cPots.stop(); 

#ifdef DIGI_POT_WRITE_VERIFICATION
	i2cPots.start(0b01011111 | I2C_WRITE, pot_pin);
	actualDigiPotValue = i2cPots.read(true, pot_pin);
	i2cPots.stop();

	i ++;
    }
    while (( i < DIGI_POT_WRITE_VERIFICATION_RETRIES ) && ( actualDigiPotValue != potValue ));
#endif
}

/// returns the last pot value set
uint8_t DigiPots::getPotValue() {
    return potValue;
}
