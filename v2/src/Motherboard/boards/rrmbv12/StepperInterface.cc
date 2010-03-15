/*
 * StepperInterface.cc
 *
 *  Created on: Mar 13, 2010
 *      Author: phooky
 */

#include "StepperInterface.hh"

void StepperInterface::setDirection(bool forward);
void StepperInterface::step();
void StepperInterface::setEnabled(bool enabled);
bool StepperInterface::isAtMaximum() {
	return max_pin.getValue();
}
bool StepperInterface::isAtMinimum();

void StepperInterface::init() {
	dir_pin.setDirection(true);
	step_pin.setDirection(true);
	enable_pin.setValue(false);
	enable_pin.setDirection(true);
	max_pin.setDirection(false);
	min_pin.setDirection(false);
}
