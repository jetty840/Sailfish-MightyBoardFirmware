/*
 * Copyright 2011 by Matt Mets <matt.mets@makerbot.com>
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

#ifndef SOFTWARE_SERVO_HH_
#define SOFTWARE_SERVO_HH_

#include "AvrPort.hh"

/**
 * Software implementation of a hobby servo driver. Though module is implemented
 * purely in software, it does require periodic servicing from a microsecond
 * capable hardware interrupt in order to function properly.
 * \ingroup SoftwareLibraries
 */
class SoftwareServo {
public:
	SoftwareServo(Pin pin);

                // Value is in degrees, from 0 to 180
	void setPosition(uint8_t position);
	void enable();
	void disable();

	bool isEnabled() { return enabled; }
	uint16_t getCounts() { return counts; }

	Pin pin;				// Pin this servo is connected to
private:
	bool enabled;			// If true, enable servo
	uint16_t counts;		// uS length of servo on-time
};

#endif
