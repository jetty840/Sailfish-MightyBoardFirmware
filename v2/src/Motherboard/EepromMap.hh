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


#ifndef EEPROMMAP_HH_
#define EEPROMMAP_HH_

#include <stdint.h>

namespace eeprom {

const static uint16_t EEPROM_SIZE				= 0x0200;

/// Version, low byte: 1 byte
const static uint16_t VERSION_LOW				= 0x0000;
/// Version, high byte: 1 byte
const static uint16_t VERSION_HIGH				= 0x0001;

// Axis inversion flags: 1 byte.
// Axis N (where X=0, Y=1, etc.) is inverted if the Nth bit is set.
const static uint16_t AXIS_INVERSION			= 0x0002;

// Endstop inversion flags: 1 byte.
// The endstops for axis N (where X=0, Y=1, etc.) are considered
// to be logically inverted if the Nth bit is set.
// Bit 7 is set to indicate endstops are present; it is zero to indicate
// that endstops are not present.
// Ordinary endstops (H21LOB et. al.) are inverted.
const static uint16_t ENDSTOP_INVERSION			= 0x0003;

// Name of this machine: 32 bytes.
const static uint16_t MACHINE_NAME				= 0x0020;

void init();

uint8_t getEeprom8(const uint16_t location, const uint8_t default_value);
uint16_t getEeprom16(const uint16_t location, const uint16_t default_value);

} // namespace eeprom

#endif // EEPROMMAP_HH_
