/*
 * MCP4451 digital potentiometer with range 0 - 255 (257 taps)
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

// Default pot wiper position is 128 (one half of full scale)
//
// On the Panucatt SureStepr SD8825 Stepper Driver v1.1, the default
// wiper position of 128 yields
//
//   VREF = 0.6V
//
// Current limit Cl is then
//
//   Cl = VREF * 2.0A / V
//
// and thus the default current is 1.2A
//
// If the digipot is set to n, 0 <= n <= 255, then the current limit Cl is
//
//   Cl(n) = (n / 256) * 2.4A
//
// To determine n given Cl, use
//
//   n(Cl) = 256 * Cl / 2.4A
//
//    Cl     n
//   (A)
//   ---   ---
//   0.1    10
//   0.2    21
//   0.3    32
//   0.4    42
//   0.5    53
//   0.6    64
//   0.7    74
//   0.8    85
//   0.9    96
//   1.0   106
//   1.1   117
//   1.2   128
//   1.3   138
//   1.4   149
//   1.5   160
//   1.6   170
//   1.7   181
//   1.8   192
//   1.9   202
//   2.0   213
//   2.1   224
//   2.2   234
//   2.3   245
//   2.4   256

#include "Compat.hh"
#include <avr/eeprom.h>
#include "Configuration.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"
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

     potValues[axis] = val;

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
