/*
 * EepromMap.cc
 *
 *  Created on: Mar 2, 2010
 *      Author: phooky
 */

#include "EepromMap.hh"
#include <avr/eeprom.h>
#include "Version.hh"

void initEeprom() {
	uint8_t version[2];
	eeprom_read_block(version,(const uint8_t*)eeprom::VERSION_LOW,2);
	if ((version[1]*100+version[0]) == firmware_version) return;
	if (version[1] == 0xff || version[1] < 2) {
		// Initialize eeprom map
		// Default: Heaters 0 and 1 enabled, thermistor on both.
		uint8_t features = eeprom::HEATER_0_PRESENT | eeprom::HEATER_0_THERMISTOR |
				eeprom::HEATER_1_PRESENT | eeprom::HEATER_1_THERMISTOR;
		eeprom_write_byte((uint8_t*)eeprom::FEATURES,features);
	}
	// Write version
	version[0] = firmware_version % 100;
	version[1] = firmware_version / 100;
	eeprom_write_block(version,(uint8_t*)eeprom::VERSION_LOW,2);
}
