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
#include <avr/eeprom.h>
#include "Version.hh"

namespace eeprom {

void init() {
	uint8_t version[2];
	eeprom_read_block(version,(const uint8_t*)eeprom::VERSION_LOW,2);
	if ((version[1]*100+version[0]) == firmware_version) return;
	if (version[1] == 0xff || version[1] < 2) {
		// Initialize eeprom map
		// Default: enstops inverted, Y axis inverted
		uint8_t axis_invert = 1<<1; // Y axis = 1
		uint8_t endstop_invert = 0b00011111; // all endstops inverted
		eeprom_write_byte((uint8_t*)eeprom::AXIS_INVERSION,axis_invert);
		eeprom_write_byte((uint8_t*)eeprom::ENDSTOP_INVERSION,endstop_invert);
		eeprom_write_byte((uint8_t*)eeprom::MACHINE_NAME,0); // name is null
	}
	// Write version
	version[0] = firmware_version % 100;
	version[1] = firmware_version / 100;
	eeprom_write_block(version,(uint8_t*)eeprom::VERSION_LOW,2);
}

uint8_t getEeprom8(const uint16_t location, const uint8_t default_value) {
	uint8_t data;
	eeprom_read_block(&data,(const uint8_t*)location,1);
	if (data == 0xff) data = default_value;
	return data;
}

uint16_t getEeprom16(const uint16_t location, const uint16_t default_value) {
	uint16_t data;
	eeprom_read_block(&data,(const uint8_t*)location,2);
	if (data == 0xffff) data = default_value;
	return data;
}

} // namespace eeprom
