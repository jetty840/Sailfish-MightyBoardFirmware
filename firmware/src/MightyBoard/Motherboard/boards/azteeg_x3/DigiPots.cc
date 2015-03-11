/*
 * MPC4451 digital potentiometer with range 0 - 255 (257 taps)
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

#include <avr/eeprom.h>
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "Configuration.hh"
#include "TWI.hh"
#include "DigiPots.hh"

#define DIGIPOT00 0b0101100
#define DIGIPOT10 0b0101110

static uint8_t potValues[STEPPER_COUNT];
static uint8_t defaultPotValues[STEPPER_COUNT];

void DigiPots::init() {
     static bool initialized = false;

     if ( initialized )
	  return;

     // Initialize the I2C hardware bus
     TWI_init();

     // Load our default VREF settings
     cli();
     eeprom_read_block(defaultPotValues,
		       (void *)eeprom_offsets::DIGI_POT_SETTINGS,
		       sizeof(uint8_t) * STEPPER_COUNT);
     sei();

     // Put the digi pots at their default VREF settings
     for (uint8_t i = 0; i < STEPPER_COUNT; i++)
	  setPotValue(i, defaultPotValues[i]);

     initialized = true;
}

uint8_t DigiPots::getPotValue(uint8_t axis) {
     // Higher level code validates the axis call argument
     return potValues[axis];
}

void DigiPots::resetPot(uint8_t axis) {
     // Higher level code validates the axis call argument
     setPotValue(axis, defaultPotValues[axis]);
}

void DigiPots::setPotValue(uint8_t axis, const uint8_t val) {
     uint8_t addr, packet[2];
     static uint8_t registers[4] = { 0x00, 0x10, 0x60, 0x70 };

     // Higher level code validates the axis call argument
     if ( axis < 4 ) {
	  addr = DIGIPOT00 << 1;
     }
     else {
	  addr = DIGIPOT10 << 1;
	  axis -= 4;
     }

     packet[0] = 0x40;
     packet[1] = 0xFF;
     TWI_write_data(addr, packet, 2);

     packet[0] = 0xA0;
     packet[1] = 0xFF;
     TWI_write_data(addr, packet, 2);

     packet[0] = registers[axis];
     packet[1] = val;
     TWI_write_data(addr, packet, 2);
}
