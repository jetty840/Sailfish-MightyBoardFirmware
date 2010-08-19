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
// Enable pin D5 is also OC0B.

int16_t last_extruder_speed;
bool stepper_motor_mode;
int16_t stepper_accumulator;
uint8_t stepper_phase;

// TIMER0 is used to PWM motor driver A enable on OC0B.
void initExtruderMotor() {
	last_extruder_speed = 0;
	TCCR0A = 0b00000011;  // Leave pin off by default
	TCCR0B = 0b00000011;
	OCR0B = 0;
	TIMSK0 = 0; // no interrupts needed.
	HB1_ENABLE_PIN.setDirection(true);
	HB1_ENABLE_PIN.setValue(false);
	HB1_DIR_PIN.setDirection(true);
	HB2_ENABLE_PIN.setDirection(true);
	HB2_ENABLE_PIN.setValue(false);
	HB2_DIR_PIN.setDirection(true);
	stepper_motor_mode = false;
	stepper_accumulator = 0;
	stepper_phase = 0;
}

void setStepperMode(bool mode) {
	stepper_motor_mode = mode;
	setExtruderMotor(last_extruder_speed);
}

void setExtruderMotor(int16_t speed) {
	if (speed == last_extruder_speed) return;
	last_extruder_speed = speed;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (!stepper_motor_mode) {
			TIMSK0 = 0;
			if (speed == 0) {
				TCCR0A = 0b00000011;
				MOTOR_ENABLE_PIN.setValue(false);
			} else {
				MOTOR_ENABLE_PIN.setValue(true);
				TCCR0A = 0b00100011;
			}
			bool backwards = speed < 0;
			if (backwards) { speed = -speed; }
			if (speed > 255) { speed = 255; }
			MOTOR_DIR_PIN.setValue(!backwards);
			OCR0B = speed;
		} else {
			TCCR0A = 0b00000000;
			TCCR0B = 0b00000011;
			TIMSK0  = 0b00000001;
		}
	}
}

const uint8_t hb1_en_pattern = 0xdd;
const uint8_t hb1_dir_pattern = 0xc3;
const uint8_t hb2_en_pattern = 0x77;
const uint8_t hb2_dir_pattern = 0xf0;

// at speed 255, ~40Hz half-stepping
const uint16_t acc_rollover = 6375;

inline void setStep() {
	const uint8_t mask = 1 << stepper_phase;
	HB1_DIR_PIN.setValue((hb1_dir_pattern & mask) != 0);
	HB1_ENABLE_PIN.setValue((hb1_en_pattern & mask) != 0);
	HB2_DIR_PIN.setValue((hb2_dir_pattern & mask) != 0);
	HB2_ENABLE_PIN.setValue((hb2_en_pattern & mask) != 0);
}

ISR(TIMER0_OVF_vect) {
	stepper_accumulator += last_extruder_speed;
	if (stepper_accumulator >= acc_rollover) {
		stepper_accumulator -= acc_rollover;
		stepper_phase = (stepper_phase + 1) % 8;
		setStep();
	} else if (stepper_accumulator < 0) {
		stepper_accumulator += acc_rollover;
		stepper_phase = (stepper_phase - 1) % 8;
		setStep();
	}
}
