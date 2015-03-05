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

#include <util/atomic.h>
#include <avr/eeprom.h>
#include "Eeprom.hh"
#include "EepromMap.hh"
#include "Configuration.hh"
#include "SoftI2cManager.hh"
#include "DigiPots.hh"
#include "StepperAxis.hh"
#include "Pin.hh"

// If defined, the digipot is read back after being written and compared against the
// written value.  If the value doesn't match, the process is repeated up t
// DIGI_POT_WRITE_VERIFICATION_RETRIES
#define DIGI_POT_WRITE_VERIFICATION

#ifdef DIGI_POT_WRITE_VERIFICATION
#define DIGI_POT_WRITE_VERIFICATION_RETRIES 5
#endif

///assume max vref is 1.95V  (allowable vref for max current rating of stepper is 1.814)
// This is incorrect it's based on 10K digipots, not 5K.
#define DIGI_POT_MAX_XYAB	118
#define DIGI_POT_MAX_Z		40

static uint8_t potValues[STEPPER_COUNT];
static uint8_t defaultPotValues[STEPPER_COUNT];
static Pin potPins[STEPPER_COUNT];

void DigiPots::init() {
     static bool initialized = false;

     if ( initialized ) return;

     SoftI2cManager::getI2cManager().init();

     cli();
     eeprom_read_block(defaultPotValues, (void *)eeprom_offsets::DIGI_POT_SETTINGS,
		       sizeof(uint8_t) * STEPPER_COUNT);
     sei();

     for (uint8_t i = 0; i < STEPPER_COUNT; i++)
	  setPotValue(i, defaultPotValues[i]);

     potPins[X_AXIS] = Pin(X_POT_PIN);
     potPins[Y_AXIS] = Pin(Y_POT_PIN);
     potPins[Z_AXIS] = Pin(Z_POT_PIN);
     potPins[A_AXIS] = Pin(A_POT_PIN);
     potPins[B_AXIS] = Pin(B_POT_PIN);

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

#if defined(DIGI_POT_WRITE_VERIFICATION)
     uint8_t i = 0, actualDigiPotValue;
     do {
#endif
	  i2cPots.start(0b01011110 | I2C_WRITE, potPins[axis]);
	  potValues[axis] = val > DIGI_POT_MAX_XYAB ? DIGI_POT_MAX_XYAB : val;
	  i2cPots.write(potValues[axis], potPins[axis]);
	  i2cPots.stop();

#if defined(DIGI_POT_WRITE_VERIFICATION)
	  i2cPots.start(0b01011111 | I2C_WRITE, potPins[axis]);
	  actualDigiPotValue = i2cPots.read(true, potPins[axis]);
	  i2cPots.stop();
	  i++;
     }
     while (( i < DIGI_POT_WRITE_VERIFICATION_RETRIES ) && ( actualDigiPotValue != potValues[axis] ));
#endif
}

