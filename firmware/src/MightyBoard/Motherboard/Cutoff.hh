/*
 * Copyright 2011 by Alison Leonard	 <alison@makerbot.com>
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

#ifndef SAFETY_CUTOFF_HH_
#define SAFETY_CUTOFF_HH_

#define CUTOFF_COUNT_LIMIT	20
#define NOISE_COUNT_LIMIT	5

#include "Configuration.hh"

class Cutoff{

public:
	void init(); // set pins to default values for cutoff // use CUTOFF_PRESENT flag to enable or disable
	
	bool isCutoffActive(); // return true if test pin is high
	
	bool noiseResponse();

private:	
	void resetCutoff();  // check if cutoff is active and strobe reset if not
	
	void disable(); // pull test line down in software, set disabled flag, check that SRlatch is low, if not call reset, else pull reset low
	
	void enable(); // turn test line to input. set enabled flag
	
	void setAlarm(); // calls piezo buzzer alarm if cutoff is triggered

	bool cutoff_enabled;
	
	uint8_t noiseCount, cutoffCount;
	bool alarmRun;
};

#endif
