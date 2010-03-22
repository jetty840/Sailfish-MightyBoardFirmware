/*
 * ExtruderBoard.cc
 *
 *  Created on: Mar 19, 2010
 *      Author: phooky
 */

#include "ExtruderBoard.hh"
#include "HeatingElement.hh"
#include "ExtruderMotor.hh"
#include "Configuration.hh"
#include <avr/interrupt.h>
#include <util/atomic.h>

ExtruderBoard ExtruderBoard::extruderBoard;

ExtruderBoard::ExtruderBoard() :
		micros(0L),
		extruder_thermistor(THERMISTOR_PIN,0),
		extruder_heater(extruder_thermistor,extruder_element)
{
}

void ExtruderBoard::reset() {
	initExtruderMotor();
	// Timer 1 is for microsecond-level timing.
	// CTC mode, interrupt on OCR1A, no prescaler
	TCCR1A = 0x00;
	TCCR1B = 0x09;
	TCCR1C = 0x00;
	OCR1A = INTERVAL_IN_MICROSECONDS * 16;
	TIMSK1 = 0x02; // turn on OCR1A match interrupt
	extruder_element.init();
	extruder_thermistor.init();
	getHostUART().enable(true);
	getHostUART().in_.reset();
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

/// Timer one comparator match interrupt
ISR(TIMER1_COMPA_vect) {
	ExtruderBoard::getBoard().doInterrupt();
}
