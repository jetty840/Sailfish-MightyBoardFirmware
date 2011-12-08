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

namespace toolhead_eeprom_offsets {
//// Start of map
//// Uninitialized memory is 0xff.  0xff should never
//// be used as a valid value for initialized memory!

//// Feature map: 2 bytes
const static uint16_t FEATURES			= 0x0000;
/// Backoff stop time, in ms: 2 bytes
const static uint16_t BACKOFF_STOP_TIME         = 0x0002;
/// Backoff reverse time, in ms: 2 bytes
const static uint16_t BACKOFF_REVERSE_TIME      = 0x0004;
/// Backoff forward time, in ms: 2 bytes
const static uint16_t BACKOFF_FORWARD_TIME      = 0x0006;
/// Backoff trigger time, in ms: 2 bytes
const static uint16_t BACKOFF_TRIGGER_TIME      = 0x0008;
/// Extruder heater base location: 6 bytes
const static uint16_t EXTRUDER_PID_BASE         = 0x000A;
/// HBP heater base location: 6 bytes data
const static uint16_t HBP_PID_BASE              = 0x0010;
/// Extra features word: 2 bytes
const static uint16_t EXTRA_FEATURES            = 0x0016;
/// Extruder identifier; defaults to 0: 1 byte 
/// Padding: 1 byte of space
const static uint16_t SLAVE_ID                  = 0x0018;
/// Cooling fan info: 2 bytes 
const static uint16_t COOLING_FAN_SETTINGS 	= 0x001A;
/// Padding: 6 empty bytes of space

// TOTAL MEMORY SIZE PER TOOLHEAD = 0x26 bytes
} 



namespace eeprom_offsets {

/// Version, low byte: 1 byte
const static uint16_t VERSION_LOW				= 0x0000;
/// Version, high byte: 1 byte
const static uint16_t VERSION_HIGH				= 0x0001;
/// Axis inversion flags: 1 byte.
/// Axis N (where X=0, Y=1, etc.) is inverted if the Nth bit is set.
/// Bit 7 is used for HoldZ OFF: 1 = off, 0 = on
const static uint16_t AXIS_INVERSION			= 0x0002;
/// Endstop inversion flags: 1 byte.
/// The endstops for axis N (where X=0, Y=1, etc.) are considered
/// to be logically inverted if the Nth bit is set.
/// Bit 7 is set to indicate endstops are present; it is zero to indicate
/// that endstops are not present.
/// Ordinary endstops (H21LOB et. al.) are inverted.
const static uint16_t ENDSTOP_INVERSION			= 0x0003;
/// Name of this machine: 32 bytes.
const static uint16_t MACHINE_NAME				= 0x0020;
/// Default locations for the axis: 5 x 32 bit = 20 bytes
const static uint16_t AXIS_HOME_POSITIONS		= 0x0060;
/// Thermistor table 0: 128 bytes
const static uint16_t THERM_TABLE		= 0x0074;
/// Padding: 8 bytes
// Toolhead 0 data: 26 bytes (see above)
const static uint16_t T0_DATA_BASE		= 0x100;
// Toolhead 0 data: 26 bytes (see above)
const static uint16_t T1_DATA_BASE		= 0x011A;
/// Digital Potentiometer Settings : 5 Bytes
const static uint16_t DIGI_POT_SETTINGS			= 0x0134;
// Padding: 1 byte free space
/// start of free space
const static uint16_t FREE_EEPROM_STARTS = 0x0140;

}

namespace therm_eeprom_offsets{
	const static uint16_t THERM_R0_OFFSET                   = 0x00;
	const static uint16_t THERM_T0_OFFSET                   = 0x04;
	const static uint16_t THERM_BETA_OFFSET                 = 0x08;
	const static uint16_t THERM_DATA_OFFSET                 = 0x10;
}

namespace eeprom_info {

const static uint16_t EEPROM_SIZE = 0x0200;


// EXTRA_FEATURES
enum {
	EF_SWAP_MOTOR_CONTROLLERS	= 1 << 0,
	EF_USE_BACKOFF			= 1 << 1,

	// Two bits to indicate mosfet channel.
	// Channel A = 0, B = 1, C = 2
	// Defaults:
	//   A - HBP heater
	//   B - extruder heater
	//   C - ABP motor
	EF_EX_HEATER_0			= 1 << 2,
	EF_EX_HEATER_1			= 1 << 3,
	EF_HBP_HEATER_0			= 1 << 4,
	EF_HBP_HEATER_1			= 1 << 5,
	EF_ABP_MOTOR_0			= 1 << 6,
	EF_ABP_MOTOR_1			= 1 << 7,

	// These are necessary to deal with horrible "all 0/all 1" problems
	// we introduced back in the day
	EF_ACTIVE_0				= 1 << 14,  // Set to 1 if EF word is valid
	EF_ACTIVE_1				= 1 << 15	// Set to 0 if EF word is valid
};


enum {
        HEATER_0_PRESENT        = 1 << 0,
        HEATER_0_THERMISTOR     = 1 << 1,
        HEATER_0_THERMOCOUPLE   = 1 << 2,

        HEATER_1_PRESENT        = 1 << 3,
        HEATER_1_THERMISTOR     = 1 << 4,
        HEATER_1_THERMOCOUPLE   = 1 << 5,

        DC_MOTOR_PRESENT                = 1 << 6,

        HBRIDGE_STEPPER                 = 1 << 8,
        EXTERNAL_STEPPER                = 1 << 9,
        RELAY_BOARD                             = 1 << 10,
        MK5_HEAD                                = 1 << 11
};


const static uint16_t EF_DEFAULT = 0x4084;



}

namespace eeprom {
	void setDefaults();
}
#endif // EEPROMMAP_HHe
