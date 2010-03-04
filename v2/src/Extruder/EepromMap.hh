#ifndef EEPROM_MAP_HH_
#define EEPROM_MAP_HH_

#include <stdint.h>
#include "Thermistor.hh"

/// Describe the EEPROM map.
/// Why are we not describing this as a packed struct?  Because the
/// information needs to be shared with external applications (currently
/// java, etc.

namespace eeprom {

const static uint16_t EEPROM_SIZE				= 0x0200;

//// Start of map

/// Version, low byte: 1 byte
const static uint16_t VERSION_LOW				= 0x0000;
/// Version, high byte: 1 byte
const static uint16_t VERSION_HIGH				= 0x0001;

//// Feature map: 2 bytes
const static uint16_t FEATURES					= 0x0002;
enum {
	HEATER_0_PRESENT		= 0,
	HEATER_0_THERMISTOR 	= 1,
	HEATER_0_THERMOCOUPLE	= 2,

	HEATER_1_PRESENT		= 3,
	HEATER_1_THERMISTOR 	= 4,
	HEATER_1_THERMOCOUPLE 	= 5,

	DC_MOTOR_PRESENT		= 6
};

/// Thermistor table 0
/// Thermistor 0 parameter r0: 4 bytes
const static uint16_t THERM_TABLE_0_R0			= 0x00f0;
/// Thermistor 0 parameter t0: 4 bytes
const static uint16_t THERM_TABLE_0_T0			= 0x00f4;
/// Thermistor 0 parameter beta: 4 bytes
const static uint16_t THERM_TABLE_0_BETA		= 0x00f8;
/// Thermistor 0 table: N bytes
const static uint16_t THERM_TABLE_0_DATA		= 0x0100;
const static uint16_t THERM_TABLE_0_END			= 0x0150;

/// Thermistor table 1
/// Thermistor 1 parameter r0: 4 bytes
const static uint16_t THERM_TABLE_1_R0			= 0x0170;
/// Thermistor 1 parameter t0: 4 bytes
const static uint16_t THERM_TABLE_1_T0			= 0x0174;
/// Thermistor 1 parameter beta: 4 bytes
const static uint16_t THERM_TABLE_1_BETA		= 0x0178;
/// Thermistor 1 table: N bytes
const static uint16_t THERM_TABLE_1_DATA		= 0x0180;
const static uint16_t THERM_TABLE_1_END			= 0x01d0;
}

void initEeprom();

#endif // EEPROM_MAP_HH_
