/*
 * StepperInterface.cc
 *
 *  Created on: Mar 13, 2010
 *      Author: phooky
 */

#include "StepperInterface.hh"
#include <util/delay_basic.h>

void StepperInterface::setDirection(bool forward) {
	dir_pin.setValue(forward);
}

void StepperInterface::step() {
	step_pin.setValue(true);
	// We need 1us of delay for the A3982.
	_delay_loop_1(5);  // 3 cycles per*5 loops @16MHz + overhead = ~1us
	step_pin.setValue(false);
}

void StepperInterface::setEnabled(bool enabled) {
	// The A3982 stepper driver chip has an inverted enable.
	enable_pin.setValue(!enabled);
}

bool StepperInterface::isAtMaximum() {
	return max_pin.getValue();
}
bool StepperInterface::isAtMinimum() {
	return min_pin.getValue();
}

void StepperInterface::init() {
	dir_pin.setDirection(true);
	step_pin.setDirection(true);
	enable_pin.setValue(true);
	enable_pin.setDirection(true);
	max_pin.setDirection(false);
	min_pin.setDirection(false);
}
