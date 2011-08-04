/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
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

#include <avr/io.h>
#include <util/atomic.h>
#include "Configuration.hh"
#include <avr/interrupt.h>
#include "ExtruderMotor.hh"
#include "Eeprom.hh"
#include "EepromMap.hh"

using namespace eeprom;

// Enable pin D5 is also OC0B.

int16_t last_extruder_speed;
bool stepper_motor_mode;
bool external_stepper_motor_mode;
int16_t stepper_accumulator;
uint8_t stepper_phase;

bool swap_motor = false;
Pin motor_enable_pin = HB1_ENABLE_PIN;
Pin motor_dir_pin = HB1_DIR_PIN;

Pin external_enable_pin = ES_ENABLE_PIN;
Pin external_dir_pin = ES_DIR_PIN;
Pin external_step_pin = ES_STEP_PIN;

// FIXME: Hardcoded steps per revolution. Eventually, this needs to be configurable
// Set to 200 for standard Makerbot Stepper Motor Driver V2.3
// Set to 5 * 200 for MakerGear 1:5 geared stepper
uint16_t extruder_steps_per_rev = 200;

volatile uint32_t ext_stepper_ticks_per_step = 0;
volatile int32_t ext_stepper_counter = 0;


// TIMER0 is used to PWM motor driver A enable on OC0B.
void initExtruderMotor() {
	last_extruder_speed = 0;
	HB1_ENABLE_PIN.setDirection(true);
	HB1_ENABLE_PIN.setValue(false);
	HB1_DIR_PIN.setDirection(true);
	HB2_ENABLE_PIN.setDirection(true);
	HB2_ENABLE_PIN.setValue(false);
	HB2_DIR_PIN.setDirection(true);
	stepper_motor_mode = false;
	stepper_accumulator = 0;
	stepper_phase = 1;
	// Check eeprom map to see if motor has been swapped to other driver chip
	uint16_t ef = getEeprom16(EXTRA_FEATURES,EF_DEFAULT);
	if ((ef & EF_SWAP_MOTOR_CONTROLLERS) != 0) {
		swap_motor = true;
		motor_enable_pin = HB2_ENABLE_PIN;
		motor_dir_pin = HB2_DIR_PIN;
	}
}

void setStepperMode(bool mode, bool external/* = false*/) {
	stepper_motor_mode = mode && !external;
	external_stepper_motor_mode = mode && external;

	if (stepper_motor_mode) {
		TCCR0A = 0;
		TCCR0B = _BV(CS01) | _BV(CS00);
		TIMSK0 = _BV(TOIE0);
	} else if (external_stepper_motor_mode) {
		// Setup pins
		external_enable_pin.setDirection(true);
		external_enable_pin.setValue(true); // true = disabled

		external_dir_pin.setDirection(true);
		external_dir_pin.setValue(true); // true = forward

		external_step_pin.setDirection(true);
		external_step_pin.setValue(false);

		// CTC Mode
		TCCR0A = _BV(WGM01);
		// Timer/Counter 0 Output Compare A Match Interrupt On
		TIMSK0  = _BV(OCIE1A);
		// 1/(16,000,000 / 8*(1+OCR0A)) = ES_TICK_LENGTH/2 micros/tick
		OCR0A = ES_TICK_LENGTH-1;
		// 8x prescaler, with CTC mode: 16MHz/8 = 2 MHz timer ticks
		TCCR0B = _BV(CS01);
	} else {
		TCCR0A = _BV(WGM01) | _BV(WGM00);  // Leave pin off by default
		TCCR0B = _BV(CS01) | _BV(CS00);
		OCR0B = 0;
		TIMSK0 = 0; // no interrupts needed.
		setExtruderMotor(last_extruder_speed);
	}
}

void setExtruderMotor(int16_t speed) {
	if (speed == last_extruder_speed) return;
	last_extruder_speed = speed;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (!stepper_motor_mode && !external_stepper_motor_mode) {
			TIMSK0 = 0;
			if (speed == 0) {
				TCCR0A = _BV(WGM01) | _BV(WGM00);
				motor_enable_pin.setValue(false);
			} else if (speed == 255) {
				TCCR0A = _BV(WGM01) | _BV(WGM00);
				motor_enable_pin.setValue(true);
			} else {
				motor_enable_pin.setValue(true);
				if (swap_motor) {
					TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00);
				} else {
					TCCR0A = _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
				}
			}
			bool backwards = speed < 0;
			if (backwards) { speed = -speed; }
			if (speed > 255) { speed = 255; }
			motor_dir_pin.setValue(!backwards);
			if (swap_motor) {
				OCR0A = speed;
			} else {
				OCR0B = speed;
			}
		}
	}
}


// set the motor's  RPM -- in microseconds for one full revolution
void setExtruderMotorRPM(uint32_t micros, bool direction) {
	// Just ignore this command if we're not using an external stepper driver
	if (!external_stepper_motor_mode) return;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (micros > 0) {
			// 60,000,000 is one RPM
			// 1,000,000 is one RPS
			ext_stepper_ticks_per_step = (micros / ES_TICK_LENGTH) / extruder_steps_per_rev;
			//ext_stepper_counter = 0;

			// Timer/Counter 0 Output Compare A Match Interrupt On
      // This is now done in setExtruderMotorOn()
      // TIMSK0  = _BV(OCIE1A);

			external_dir_pin.setValue(direction); // true = forward
			external_enable_pin.setValue(false); // true = disabled
			external_step_pin.setValue(false);
			// DEBUG_LED.setValue(true);
		} else {
			// Timer/Counter 0 Output Compare A Match Interrupt Off
			TIMSK0  = 0;
			external_enable_pin.setValue(true); // true = disabled
			ext_stepper_ticks_per_step = 0;
			// DEBUG_LED.setValue(false);
		}
	}

}

#ifdef DEFAULT_EXTERNAL_STEPPER
void setExtruderMotorOn(bool on)
{
	if (!external_stepper_motor_mode) return;
  // Disable stepping but hold torque by disabling interrupt
	if (on) {
		TIMSK0  = _BV(OCIE1A);
	} else {
		TIMSK0  = 0;
	}
}
#endif

// ## H-Bridge Stepper Driving using Timer0 Overflow ##

const uint8_t hb1_en_pattern = 0xdd;
const uint8_t hb1_dir_pattern = 0xc3;
const uint8_t hb2_en_pattern = 0x77;
const uint8_t hb2_dir_pattern = 0xf0;

// at speed 255, ~80Hz full-stepping
const int16_t acc_rollover = (6375/2);

volatile uint8_t stepper_pwm = 0;

inline void setStep() {
	const bool enable = (last_extruder_speed != 0) && (((stepper_pwm++) & 0x01) == 0);
	const uint8_t mask = 1 << stepper_phase;
	HB1_DIR_PIN.setValue((hb1_dir_pattern & mask) != 0);
	HB1_ENABLE_PIN.setValue( enable && ((hb1_en_pattern & mask) != 0) );
	HB2_DIR_PIN.setValue((hb2_dir_pattern & mask) != 0);
	HB2_ENABLE_PIN.setValue( enable && ((hb2_en_pattern & mask) != 0) );
}

ISR(TIMER0_OVF_vect) {
	stepper_accumulator += last_extruder_speed;
	if (stepper_accumulator >= acc_rollover) {
		stepper_accumulator -= acc_rollover;
		stepper_phase = (stepper_phase + 2) & 0x07;
	} else if (stepper_accumulator < 0) {
		stepper_accumulator += acc_rollover;
		stepper_phase = (stepper_phase - 2) & 0x07;
	}
	setStep();
}

// ## External Stepper Driving using Timer 0 Compare A ##

ISR(TIMER0_COMPA_vect) {
	if (ext_stepper_ticks_per_step > 0) {
		++ext_stepper_counter;
		if (ext_stepper_counter >= ext_stepper_ticks_per_step) {
			external_step_pin.setValue(true);
			ext_stepper_counter -= ext_stepper_ticks_per_step;
			external_step_pin.setValue(false);
		}
	}
}
