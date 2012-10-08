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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
    SoftI2cManager i2cPots = SoftI2cManager::getI2cManager();
#pragma GCC diagnostic pop
    i2cPots.start(0b01011110 | I2C_WRITE, pot_pin);
    potValue = eeprom::getEeprom8(eeprom_base + eeprom_pot_offset, 0);
    i2cPots.write(potValue, pot_pin);
    i2cPots.stop();
}

void DigiPots::setPotValue(const uint8_t val)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
    SoftI2cManager i2cPots = SoftI2cManager::getI2cManager();
#pragma GCC diagnostic pop
    i2cPots.start(0b01011110 | I2C_WRITE, pot_pin);
    potValue = val > DIGI_POT_MAX ? DIGI_POT_MAX : val;
    i2cPots.write(potValue, pot_pin);
    i2cPots.stop(); 
}

/// returns the last pot value set
uint8_t DigiPots::getPotValue() {
    return potValue;
}

