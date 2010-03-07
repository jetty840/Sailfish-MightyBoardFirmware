/*
 * ExtruderMotor.cc
 *
 *  Created on: Mar 7, 2010
 *      Author: phooky
 */
#include <avr/io.h>
#include "Configuration.hh"

// Enable pin D5 is also OC0B.

// TIMER0 is used to PWM motor driver A enable on OC0B.
void initExtruderMotor() {
	TCCR0A = 0b00100011;
	TCCR0B = 0b00000001;
	OCR0B = 0;
	TIMSK0 = 0; // no interrupts needed.
	MOTOR_ENABLE_PIN.setDirection(true);
	MOTOR_DIR_PIN.setDirection(true);
}

void setExtruderMotor(int16_t speed) {
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
	OCR0A = speed;
}
