/*
 * HeatingElement.cc
 *
 *  Created on: Feb 24, 2010
 *      Author: phooky
 */

#include "HeatingElement.hh"
#include <avr/sfr_defs.h>
#include <avr/io.h>

// TIMER2 is used to PWM mosfet channel B on OC2A.
void initHeatingElement() {
	TCCR2A = 0b10000011;
	TCCR2B = 0b00000110;
	OCR2A = 0;
	TIMSK2 = 0; // no interrupts needed.
}

void setHeatingElement(uint8_t value) {
	OCR2A = value;
}
