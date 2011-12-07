/*
 * Copyright 2010 by Adam Mayer <adam@makerbot.com>
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

// TODO: Shouldn't this just reset everything to an uninitialized state?

void setDefaults() {
    // Initialize eeprom map
    // Default: enstops inverted, Y axis inverted
    uint8_t axis_invert = 1<<1; // Y axis = 1
    uint8_t endstop_invert = 0b00011111; // all endstops inverted
    uint8_t features = eeprom_info::HEATER_0_PRESENT |	eeprom_info::HEATER_0_THERMISTOR; //FAR HACK. I believe this is right, but it's not clear.
//                        eeprom::HEATER_1_PRESENT |
//                        eeprom::HEATER_1_THERMISTOR;
    eeprom_write_byte((uint8_t*)eeprom_offsets::AXIS_INVERSION, axis_invert);
    eeprom_write_byte((uint8_t*)eeprom_offsets::ENDSTOP_INVERSION, endstop_invert);
    //eeprom_write_byte((uint8_t*)eeprom_offsets::MACHINE_NAME,(uint8_t)"The Replicator"); //test check. do not ship
    eeprom_write_block((uint8_t*)eeprom_offsets::MACHINE_NAME,(void*)"",sizeof("")); // name is null
    eeprom_write_byte((uint8_t*)(eeprom_offsets::T0_DATA_BASE + toolhead_eeprom_offsets::FEATURES), features);
    eeprom_write_byte((uint8_t*)(eeprom_offsets::T1_DATA_BASE + toolhead_eeprom_offsets::FEATURES), features);
}

}
