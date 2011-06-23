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

#include "ExtruderBoard.hh"
#include "HeatingElement.hh"
#include "ExtruderMotor.hh"
#include "MotorController.hh"
#include "Configuration.hh"
#include "CoolingFan.hh"
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/sfr_defs.h>
#include <avr/io.h>

ExtruderBoard ExtruderBoard::extruder_board;

ExtruderBoard::ExtruderBoard() :
		micros(0L),
		extruder_thermocouple(THERMOCOUPLE_CS,THERMOCOUPLE_SCK,THERMOCOUPLE_SO),
		platform_thermistor(PLATFORM_PIN,1),
		extruder_heater(extruder_thermocouple,extruder_element,SAMPLE_INTERVAL_MICROS_THERMOCOUPLE,eeprom::EXTRUDER_PID_P_TERM),
		platform_heater(platform_thermistor,platform_element,SAMPLE_INTERVAL_MICROS_THERMISTOR,eeprom::HBP_PID_P_TERM),
		using_platform(true),
		servoA(SERVO0),
		servoB(SERVO1),
		coolingFan(extruder_heater)
{
}

// Get the reset flags from the processor, as a bitfield
// return: The bitfield looks like this: 0 0 0 0 WDRF BORF EXTRF PORF
uint8_t ExtruderBoard::getResetFlags() {
	return resetFlags;
}

// Turn on/off PWM for channel A on OC1B
void pwmAOn(bool on) {
	if (on) {
		TCCR1A |= 0b00100000;
	} else {
		TCCR1A &= 0b11001111;
	}
}

// Turn on/off PWM for channel B on OC1A
void pwmBOn(bool on) {
	if (on) {
		TCCR1A |= 0b10000000;
	} else {
		TCCR1A &= 0b00111111;
	}
}

// Turn on/off PWM for channel C on OC0A
void pwmCOn(bool on) {
	if (on) {
		TCCR0A |= 0b10000000;
	} else {
		TCCR0A &= 0b00111111;
	}
}

void ExtruderBoard::reset(uint8_t resetFlags) {
	this->resetFlags = resetFlags;

	initExtruderMotor();

	servoA.disable();
	servoB.disable();

	// Set the output mode for the mosfets.  All three should default
	// off.
	CHANNEL_A.setValue(false);
	CHANNEL_A.setDirection(true);
	CHANNEL_B.setValue(false);
	CHANNEL_B.setDirection(true);
	CHANNEL_C.setValue(false);
	CHANNEL_C.setDirection(true);

	// Timer 0:
	//  Mode: Phase-correct PWM (WGM2:0 = 001), cycle freq= 976 Hz
	//  Prescaler: 1/64 (250 KHz)
	//  Mosfet C (labeled heater, used for extruder heater)
	//   - uses OCR0A to generate PWM
	//  H-bridge enable (used for DC motor, or fan on stepstruder:
	//   - uses OCR0B to generate PWM
	TCCR0A = 0b00000001;
	TCCR0B = 0b00000011;
	OCR0A = 0;
	OCR0B = 0;
	TIMSK0 = 0b00000000; // no interrupts needed

	// Timer 1:
	//  Mode: Normal (WGM13:0 = 0000), cycle freq= 30Hz
	//  Prescaler: 1/8 (2 MHz)
	//  Mosfet A (labeled fan, used for ABP motor)
	//  - Uses  OCR1B to generate PWM
	//  Mosfet B (labeled extra, used for HBP heater)
	//  - Uses  OCR1A to generate PWM
	TCCR1A = 0b00000000;
	TCCR1B = 0b00000010;
	OCR1A = 0;
	OCR1B = 0;
	TIMSK0 = 0b00000000; // no interrupts needed


	// Timer 2:
	//  Mode: CTC (WGM2:0 = 010), cycle freq=
	//  Prescaler: 1/32 (500 KHz)
	//  used as a provider for microsecond-level counting
	//  - Generates interrupt every 32uS
	//  used also to run servos in software
	TCCR2A = 0x02; // CTC is mode 2 on timer 2
	TCCR2B = 0x03; // prescaler: 1/32
	OCR2A = INTERVAL_IN_MICROSECONDS / 2; // 2uS/tick at 1/32 prescaler
	TIMSK2 = 0x02; // turn on OCR2A match interrupt


	extruder_thermocouple.init();
	platform_thermistor.init();
	extruder_heater.reset();
	platform_heater.reset();
	setMotorSpeed(0);
	getHostUART().enable(true);
	getHostUART().in.reset();

	coolingFan.reset();

//        flashIndicatorLED();
}

void ExtruderBoard::setMotorSpeed(int16_t speed) {
	// Since the motor and regulated cooling fan share an output, only one can be enabled at a time.
	// Therefore, we should override the motor speed command if the cooling fan is activated.
	if (!coolingFan.isEnabled()) {
		setExtruderMotor(speed);
	}
}

void ExtruderBoard::setServo(uint8_t index, int value) {
	SoftwareServo* servo;
	if (index == 0) {
		servo = &servoA;
	}
	else if (index == 1) {
		servo = &servoB;
	}
	else {
		return;
	}

	if (value == -1) {
		servo->disable();
	}
	else {
		if (!(servo->isEnabled())) {
			servo->enable();
		}
		servo->setPosition(value);
	}
}

micros_t ExtruderBoard::getCurrentMicros() {
	micros_t micros_snapshot;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		micros_snapshot = micros;
	}
	return micros_snapshot;
}

/// Run the extruder board interrupt
void ExtruderBoard::doInterrupt() {
	static micros_t servo_counter = 0;

	micros += INTERVAL_IN_MICROSECONDS;

	// Check if the servos need servicing
	servo_counter += INTERVAL_IN_MICROSECONDS;

	// Overflow, so turn both servos on
	if (servo_counter > 16000) {
		servo_counter = 0;

		if (servoA.isEnabled()) {
			servoA.pin.setValue(true);
		}
		if (servoB.isEnabled()) {
			servoB.pin.setValue(true);
		}

		// TODO: Make this its own timer? It /should/ be a slice
		coolingFan.manageCoolingFan();
	}

	if ((servoA.isEnabled()) && (servo_counter > servoA.getCounts())) {
		servoA.pin.setValue(false);
	}
	if ((servoB.isEnabled()) && (servo_counter > servoB.getCounts())) {
		servoB.pin.setValue(false);
	}
}

void ExtruderBoard::setFan(bool on) {
	CHANNEL_A.setValue(on);
}

void ExtruderBoard::setValve(bool on) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		setUsingPlatform(false);
		pwmBOn(false);
		CHANNEL_B.setValue(on);
	}
}

void ExtruderBoard::indicateError(int errorCode) {
	// The debug LED must never be set, because... IT IS CONNECTED TO THE SCK PIN.  *slow clap*
	//DEBUG_LED.setValue(errorCode != 0);
}

void ExtruderBoard::lightIndicatorLED() {
    MOTOR_DIR_PIN.setValue(true);
}

void ExtruderBoard::setUsingPlatform(bool is_using) {
	using_platform = is_using;
}

/// Timer two comparator A match interrupt
ISR(TIMER2_COMPA_vect) {
	ExtruderBoard::getBoard().doInterrupt();
}

void ExtruderHeatingElement::setHeatingElement(uint8_t value) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (value == 0 || value == 255) {
			pwmCOn(false);
			CHANNEL_C.setValue(value == 255);
		} else {
			OCR0A = value;
			pwmCOn(true);
		}
	}
}

void BuildPlatformHeatingElement::setHeatingElement(uint8_t value) {
	// This is a bit of a hack to get the temperatures right until we fix our
	// PWM'd PID implementation.  We reduce the MV to one bit, essentially.
	// It works relatively well.
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		pwmBOn(false);
		CHANNEL_B.setValue(value != 0);
	}
}
