/*
 * StepperInterface.hh
 *
 *  Created on: Mar 11, 2010
 *      Author: phooky
 */

#ifndef STEPPERINTERFACE_HH_
#define STEPPERINTERFACE_HH_

#include <AvrPort.hh>

class StepperInterface {
public:
	void setDirection(bool forward);
	void step();
	void setEnabled(bool enabled);
	bool isAtMaximum();
	bool isAtMinimum();

private:
	void init();

	friend class Motherboard;
	Pin dir_pin;
	Pin step_pin;
	Pin enable_pin;
	Pin max_pin;
	Pin min_pin;
	StepperInterface() {}
	StepperInterface(const Pin& dir,
			const Pin& step,
			const Pin& enable,
			const Pin& max,
			const Pin& min) :
				dir_pin(dir),
				step_pin(step),
				enable_pin(enable),
				max_pin(max),
				min_pin(min) {}
};

#endif /* STEPPERINTERFACE_HH_ */
