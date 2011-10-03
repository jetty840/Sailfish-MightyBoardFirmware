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

#include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>

namespace eeprom {

void setDefaults() {
        // Initialize eeprom map
        // Default: Heaters 0 and 1 enabled, thermistor on both.
        uint8_t features = eeprom::HEATER_0_PRESENT |
                        eeprom::HEATER_0_THERMISTOR |
                        eeprom::HEATER_1_PRESENT |
                        eeprom::HEATER_1_THERMISTOR;
        eeprom_write_byte((uint8_t*)eeprom::FEATURES,features);
}

} // namespace eeprom
