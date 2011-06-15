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

#ifndef COOLING_FAN_HH_
#define COOLING_FAN_HH_

#include "Heater.hh"

class CoolingFan {
public:
	CoolingFan(Heater heater);
	void setSetpoint(int temperature);
	void enable();
	void disable();

	bool isEnabled() { return enabled; }
	int getSetpoint() { return setPoint; }

	void reset();

	// Call once each cooling fan update interval
	void manageCoolingFan();

private:
	void enableFan();
	void disableFan();

	Heater heater;

	bool enabled;
	int setPoint;
};

#endif
