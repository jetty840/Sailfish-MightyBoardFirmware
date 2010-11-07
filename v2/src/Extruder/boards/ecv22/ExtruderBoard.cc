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

// channel choices
typedef enum {
	CHA =0, CHB =1, CHC =2
} ChannelChoice;

ChannelChoice heater_channel = CHB;
ChannelChoice hbp_channel = CHA;
ChannelChoice abp_channel = CHC;

volatile bool using_relays = false;

ExtruderBoard::ExtruderBoard() :
		micros(0L),
		extruder_thermistor(THERMISTOR_PIN,0),
		platform_thermistor(PLATFORM_PIN,1),
		extruder_heater(extruder_thermistor,extruder_element,SAMPLE_INTERVAL_MICROS_THERMISTOR,eeprom::EXTRUDER_PID_P_TERM),
		platform_heater(platform_thermistor,platform_element,SAMPLE_INTERVAL_MICROS_THERMISTOR,eeprom::HBP_PID_P_TERM),
		using_platform(true)
{
	// Check eeprom map to see if motor has been swapped to other driver chip
	uint16_t ef = eeprom::getEeprom16(eeprom::EXTRA_FEATURES,eeprom::EF_DEFAULT);
	heater_channel = (ChannelChoice)((ef >> 2) & 0x03);
	hbp_channel = (ChannelChoice)((ef >> 4) & 0x03);
	abp_channel = (ChannelChoice)((ef >> 6) & 0x03);
}

// Turn on/off PWM for channel A.
void pwmAOn(bool on) {
	if (on) {
		TIMSK2 = 0b00000101;
	} else {
		TIMSK2 = 0b00000000;
	}
}

// Turn on/off PWM for channel B.
void pwmBOn(bool on) {
	if (on) {
		TCCR2A = 0b10000011;
	} else {
		TCCR2A = 0b00000011;
	}
}

#if !defined(DEFAULT_EXTERNAL_STEPPER)
#define SERVO_COUNT 1
#else
#define SERVO_COUNT 2
#endif

volatile int servoPos[SERVO_COUNT];

// Index 0 = D9, Index 1 = D10.  Value = -1 to turn off, 0-255 to set position.
void ExtruderBoard::setServo(uint8_t index, int value) {
	// -2 == disabled, and once disabled it can't be set again
	if (servoPos[index] != -2)
		servoPos[index] = value;
	
	if (value = -2) {
		// diable a servo, turn off it's timer
		TIMSK1 = _BV(ICIE1) | (servoPos[0] != -2 ? _BV(OCIE1A) : 0) | (servoPos[1] != -2 ? _BV(OCIE1B) : 0);
	}
}

void ExtruderBoard::reset() {
	for (uint8_t i = 0; i < SERVO_COUNT; i++) {
		servoPos[i] = -1;
	}
	initExtruderMotor();
	// Timer 1 is for microsecond-level timing and servo pulses.
	// CTC mode, interrupt on ICR1, top at ICR1, no prescaler
	TCCR1A = _BV(WGM11);
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
	TCCR1C = 0x00;
	ICR1 = INTERVAL_IN_MICROSECONDS * 16;
	TIMSK1 = _BV(ICIE1) | _BV(OCIE1A) | _BV(OCIE1B); // turn on ICR1 match interrupt
	TIMSK1 = _BV(ICIE1) | _BV(OCIE1A); // turn on ICR1 match interrupt
	TIMSK2 = 0x00; // turn off channel A PWM by default
	// TIMER2 is used to PWM mosfet channel B on OC2A, and channel A on
	// PC1 (using the OC2B register).
	DEBUG_LED.setDirection(true);
	CHANNEL_A.setValue(false);
	CHANNEL_A.setDirection(true); // set channel A as output
	CHANNEL_B.setValue(false);
	CHANNEL_B.setDirection(true); // set channel B as output
	CHANNEL_C.setValue(false);
	CHANNEL_C.setDirection(true); // set channel C as output
	TCCR2A = 0b10000011;
	TCCR2B = 0b00000110; // prescaler 1/256
	OCR2A = 0;
	OCR2B = 0;
	// We use interrupts on OC2B and OVF to control channel A.
	TIMSK2 = 0b00000101;
	extruder_thermistor.init();
	platform_thermistor.init();
	extruder_heater.reset();
	platform_heater.reset();
	setMotorSpeed(0);
	getHostUART().enable(true);
	getHostUART().in.reset();

/*
	// These are disabled until the newer replicatorg with eeprom path
	// support has been out for a while.
	uint16_t features = getEeprom16(eeprom::FEATURES);
	setUsingRelays((features & eeprom::RELAY_BOARD) != 0);
	setStepperMode((features & (eeprom::HBRIDGE_STEPPER | eeprom::EXTERNAL_STEPPER)) != 0, (features & eeprom::EXTERNAL_STEPPER) != 0);

	if ((features & eeprom::EXTERNAL_STEPPER) == 0) {
		// Init servo ports: OC1A and OC1B as outputs when not linked to counter.
		PORTB &= ~_BV(1);
		DDRB |= _BV(1);
	} else {
		// Init servo ports: OC1A and OC1B as outputs when not linked to counter.
		PORTB &= ~_BV(1) & ~_BV(2);
		DDRB |= _BV(1) | _BV(2);
	}
*/
	
#if defined DEFAULT_STEPPER
#warning Using internal stepper!
	setStepperMode(true, false);
	// Init servo ports: OC1A and OC1B as outputs when not linked to counter.
	PORTB &= ~_BV(1) & ~_BV(2);
	DDRB |= _BV(1) | _BV(2);
#elif defined DEFAULT_EXTERNAL_STEPPER
#warning Using external stepper!
	setStepperMode(true, true);
	// Init servo ports: OC1A and OC1B as outputs when not linked to counter.
	PORTB &= ~_BV(1); // We don't use D10 for a servo with external steppers, it's the enable pin
	DDRB |= _BV(1); // but it's still an output
#else
#warning Using DC Motor!
	setStepperMode(false);
	// Init servo ports: OC1A and OC1B as outputs when not linked to counter.
	PORTB &= ~_BV(1) & ~_BV(2);
	DDRB |= _BV(1) | _BV(2);
#endif

#ifdef DEFAULT_RELAYS
	setUsingRelays(true);
#else
	setUsingRelays(false);
#endif
}

void ExtruderBoard::setMotorSpeed(int16_t speed) {
	setExtruderMotor(speed);
}

void ExtruderBoard::setMotorSpeedRPM(uint32_t speed, bool direction) {
	setExtruderMotorRPM(speed, direction);
}

micros_t ExtruderBoard::getCurrentMicros() {
	micros_t micros_snapshot;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		micros_snapshot = micros;
	}
	return micros_snapshot;
}

// ms between servo updates; conservative to avoid 7805 overheating
#define SERVO_CYCLE_LENGTH 8
volatile uint8_t servo_cycle = 0;

/// Run the extruder board interrupt
void ExtruderBoard::doInterrupt() {
	// update microseconds
	micros += INTERVAL_IN_MICROSECONDS;
	// update servos
	ExtruderBoard::getBoard().indicateError(0);
	if (servo_cycle == 0) {
		if (servoPos[0] >= 0) {
			PORTB |= _BV(1);
			OCR1A = (600*16) + (servoPos[0]*160);
		}
		
		// figure out how to make this runtime settable...
#if defined(DEFAULT_EXTERNAL_STEPPER)
		if (servoPos[1] != -1) {
			PORTB |= _BV(2);
			OCR1B = (600*16) + (servoPos[1] * 160);
		}
#endif
	}
	servo_cycle++;
	if (servo_cycle > SERVO_CYCLE_LENGTH) { servo_cycle = 0; }
}


void setChannel(ChannelChoice c, uint8_t value, bool binary) {
	if (c == CHA) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			if (binary) {
				pwmAOn(false);
				CHANNEL_A.setValue(value != 0);
			} else if (value == 0 || value == 255) {
				pwmAOn(false);
				CHANNEL_A.setValue(value == 255);
			} else {
				OCR2B = value;
				pwmAOn(true);
			}
		}
	} else if (c == CHB) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			if (binary) {
				pwmBOn(false);
				CHANNEL_B.setValue(value != 0);
			} else if (value == 0 || value == 255) {
				pwmBOn(false);
				CHANNEL_B.setValue(value == 255);
			} else {
				OCR2A = value;
				pwmBOn(true);
			}
		}
	} else {
		// channel C -- no pwm
		CHANNEL_C.setValue(value == 0?false:true);
	}
}

void ExtruderBoard::setFan(bool on) {
	setChannel(abp_channel,on?255:0,true);
}

// When using as a valve driver, always use channel A, regardless of
// extruder heat settings.
void ExtruderBoard::setValve(bool on) {
	setUsingPlatform(false);
	setChannel(CHA,on?255:0,true);
}

void ExtruderBoard::indicateError(int errorCode) {
	DEBUG_LED.setValue(errorCode != 0);
}

void ExtruderBoard::setUsingPlatform(bool is_using) {
	using_platform = is_using;
}

void ExtruderBoard::setUsingRelays(bool is_using) {
	using_relays = is_using;
}

/// Timer one ICR1 match interrupt
ISR(TIMER1_CAPT_vect) {
	ExtruderBoard::getBoard().doInterrupt();
}

// D9 - stepper 1
ISR(TIMER1_COMPA_vect) {
	PORTB &= ~_BV(1);
}

// D10 - stepper 2
ISR(TIMER1_COMPB_vect) {
	PORTB &= ~_BV(2);
}

void ExtruderHeatingElement::setHeatingElement(uint8_t value) {
	setChannel(heater_channel,value,using_relays);
}

void BuildPlatformHeatingElement::setHeatingElement(uint8_t value) {
	// This is a bit of a hack to get the temperatures right until we fix our
	// PWM'd PID implementation.  We reduce the MV to one bit, essentially.
	// It works relatively well.
	setChannel(hbp_channel,value,true);
}

ISR(TIMER2_OVF_vect) {
	if (OCR2B != 0) {
		CHANNEL_A.setValue(true);
	}
}

ISR(TIMER2_COMPB_vect) {
	CHANNEL_A.setValue(false);
}
