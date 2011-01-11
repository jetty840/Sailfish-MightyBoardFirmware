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
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/sfr_defs.h>
#include <avr/io.h>
#include "EepromMap.hh"

ExtruderBoard ExtruderBoard::extruderBoard;

ExtruderBoard::ExtruderBoard() :
		micros(0L),
		extruder_thermocouple(THERMOCOUPLE_CS,THERMOCOUPLE_SCK,THERMOCOUPLE_SO),
		platform_thermistor(PLATFORM_PIN,1),
		extruder_heater(extruder_thermocouple,extruder_element,SAMPLE_INTERVAL_MICROS_THERMOCOUPLE,eeprom::EXTRUDER_PID_P_TERM),
		platform_heater(platform_thermistor,platform_element,SAMPLE_INTERVAL_MICROS_THERMISTOR,eeprom::HBP_PID_P_TERM),
		using_platform(true)
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

void ExtruderBoard::reset() {
	resetFlags = MCUSR & 0x0f;

	initExtruderMotor();

	// Set the output mode for the mosfets.  All three should default
	// off.
	CHANNEL_A.setValue(false);
	CHANNEL_A.setDirection(true);
	CHANNEL_B.setValue(false);
	CHANNEL_B.setDirection(true);
	CHANNEL_C.setValue(false);
	CHANNEL_C.setDirection(true);

	// Timer 2 is for microsecond-level timing.
	// CTC mode, interrupt on OC2A, no prescaler
	TCCR2A = 0x02; // CTC is mode 2 on timer 2
	TCCR2B = 0x03; // prescaler: 1/32
	OCR2A = INTERVAL_IN_MICROSECONDS / 2; // 2uS/tick at 1/32 prescaler
	TIMSK2 = 0x02; // turn on OCR2A match interrupt

	// Timer 1 is for PWM of mosfet channel B on OC1A/PB1, and
	// mosfet channel A on OC1B/PB2.
	// The mode is 0101, which is 8-bit rollover PWM, yay convenience.
	// A 1/64 prescaler (CS 011) gives us a PWM cycle of 1/16ms.
	// Start with both mosfets off.
	TCCR1A = 0b00000001;
	TCCR1B = 0b00001011;
	OCR1A = 0;
	OCR1B = 0;
	TIMSK1 = 0b00000000; // no interrupts needed
	extruder_thermocouple.init();
	platform_thermistor.init();
	extruder_heater.reset();
	platform_heater.reset();
	setMotorSpeed(0);
	getHostUART().enable(true);
	getHostUART().in.reset();
}

void ExtruderBoard::setMotorSpeed(int16_t speed) {
	setExtruderMotor(speed);
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
	micros += INTERVAL_IN_MICROSECONDS;
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
	DEBUG_LED.setValue(errorCode != 0);
}

void ExtruderBoard::setUsingPlatform(bool is_using) {
	using_platform = is_using;
}

/// Timer one comparator match interrupt
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
