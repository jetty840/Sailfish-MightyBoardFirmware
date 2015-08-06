/*
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

#include "Compat.hh"
#include <avr/eeprom.h>
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "Configuration.hh"
#include "SoftI2cManager.hh"
#include "DigiPots.hh"
#include "StepperAxis.hh"
#include "Pin.hh"

// If defined, the digipot is read back after being written and compared
// against the written value.  If the value doesn't match, the process is
// repeated up to DIGI_POT_WRITE_VERIFICATION_RETRIES
#define DIGI_POT_WRITE_VERIFICATION

#ifdef DIGI_POT_WRITE_VERIFICATION
#define DIGI_POT_WRITE_VERIFICATION_RETRIES 5
#endif

// MBI's calcs are all wrong as they used a 10K pot value while they built
// their boards with 5K pots.  So, I've scrapped their comments for their
// equivalent code (of which this is a complete rewrite).
//
// At pot setting 127, the maximum voltage is
//
//    1.67V = (5K / (5K + 10K)) * 5.00V
//
// The resistance, Rs, of the sensor resistor on the botstep is
//
//    Rs = 0.27 Ohms
//
// According to the spec sheet for the A4928, the current limit
// iTripMax is,
//
//    iTripMax = Vref / (8 x Rs)
//
// Thus we can generate a nifty table of digipot settings, n, and
// the resulting current limit using
//
//     Vref(n) = 1.67V * (n / 127), 0 <= n <= 127,
//     iTripMax(n) = 1.67V * (n / 127) / (8 x 0.27 Ohms)
//
//   Digipot  iTripMax (A)
//   ---------------------
//     10        0.06 A
//     20        0.12
//     30        0.18
//     40        0.24
//     50        0.30
//     60        0.36
//     70        0.43
//     80        0.49
//     90        0.55
//    100        0.61
//    110        0.67
//    118        0.72
//    120        0.73
//    127        0.77

#define DIGI_POT_MAX_XYAB	118
#define DIGI_POT_MAX_Z		40

static uint8_t potValues[STEPPER_COUNT];
static uint8_t defaultPotValues[STEPPER_COUNT];
static Pin potPins[STEPPER_COUNT];

void DigiPots::init() {
     static bool initialized = false;

     if ( initialized )
	  return;

     // Initialize I2C bit-banger
     SoftI2cManager::getI2cManager().init();

     potPins[X_AXIS] = X_POT_PIN;
     potPins[Y_AXIS] = Y_POT_PIN;
     potPins[Z_AXIS] = Z_POT_PIN;
     potPins[A_AXIS] = A_POT_PIN;
     potPins[B_AXIS] = B_POT_PIN;

     cli();
     eeprom_read_block(defaultPotValues,
		       (void *)eeprom_offsets::DIGI_POT_SETTINGS,
		       sizeof(uint8_t) * STEPPER_COUNT);
     sei();

     initialized = true;
}

uint8_t DigiPots::getPotValue(uint8_t axis) {
     // Higher level code validates axis
     return potValues[axis];
}

void DigiPots::resetPot(uint8_t axis) {
     // Higher level code validates axis
     setPotValue(axis, defaultPotValues[axis]);
}

void DigiPots::setPotValue(uint8_t axis, const uint8_t val) {
     // Higher level code validates axis
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
     SoftI2cManager i2cPots = SoftI2cManager::getI2cManager();
#pragma GCC diagnostic pop

     potValues[axis] = val > DIGI_POT_MAX_XYAB ? DIGI_POT_MAX_XYAB : val;

#if defined(DIGI_POT_WRITE_VERIFICATION)
     uint8_t i = 0, actualDigiPotValue;
     do {
#endif
	  i2cPots.start(0b01011110 | I2C_WRITE, potPins[axis]);
	  i2cPots.write(potValues[axis], potPins[axis]);
	  i2cPots.stop();

#if defined(DIGI_POT_WRITE_VERIFICATION)
	  i2cPots.start(0b01011111 | I2C_WRITE, potPins[axis]);
	  actualDigiPotValue = i2cPots.read(true, potPins[axis]);
	  i2cPots.stop();
	  i++;
     }
     while (( i < DIGI_POT_WRITE_VERIFICATION_RETRIES ) &&
	    ( actualDigiPotValue != potValues[axis] ));
#endif
}

