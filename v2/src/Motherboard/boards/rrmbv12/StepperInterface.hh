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

#ifndef BOARDS_RRMBV12_STEPPERINTERFACE_HH_
#define BOARDS_RRMBV12_STEPPERINTERFACE_HH_

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

#endif // BOARDS_RRMBV12_STEPPERINTERFACE_HH_
