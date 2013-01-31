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

#include "Cutoff.hh"
#include "Pin.hh"
#include "Piezo.hh"


void Cutoff::init()
{
	// reset pin is an output, default low
	CUTOFF_RESET.setValue(false);
	CUTOFF_RESET.setDirection(true);
	
	// SR Check pin is an input, checks if safety cutoff is active
	CUTOFF_SR_CHECK.setValue(false);
	CUTOFF_SR_CHECK.setDirection(false);
 
	if (CUTOFF_PRESENT)
		enable();
	else
		disable();
		
	noiseCount = 0;
	cutoffCount = 0;
	alarmRun = false;
}
bool Cutoff::isCutoffActive()
{
	// if cutoff test pin is high or if cutoff output is latched high, flag as cutoff active
	if(CUTOFF_TEST.getValue() || CUTOFF_SR_CHECK.getValue())
		return true;
	else
		return false;
}

void Cutoff::enable()
{
	// cutoff test pin is an input
	CUTOFF_TEST.setValue(false);
	CUTOFF_TEST.setDirection(false);
	
	// set enabled flag
	cutoff_enabled = true;
	
	// if output is shut off (high when off), toggle reset pin
	if(CUTOFF_SR_CHECK.getValue())
		resetCutoff();
	
	// set reset line to default value (low)
	CUTOFF_RESET.setValue(false);
	
}
void Cutoff::disable()
{
	// cutoff test pin is an output and pulled down
	CUTOFF_TEST.setValue(false);
	CUTOFF_TEST.setDirection(true);
	
	cutoff_enabled = false;
	// if output is shut off (high when off), toggle reset pin
	if(CUTOFF_SR_CHECK.getValue())
		resetCutoff();
	
	// set reset line to default value (low)
	CUTOFF_RESET.setValue(false);
	
}

void Cutoff::resetCutoff(){
	
	//Timeout resetTimeout;

	// abort if cutoff test is high
	if(CUTOFF_TEST.getValue())
		return;
		
	// toggle reset pin	
	CUTOFF_RESET.setValue(true);
	
	// ensure that reset worked
	// if not, enable hardware circuit
	if(CUTOFF_SR_CHECK.getValue())
		disable();
	// if reset OK, return reset line to default (pulled down)
	else
		CUTOFF_RESET.setValue(false);
}

// call piezo alarm in cutoff is triggered
void Cutoff::setAlarm()
{
	if(!alarmRun)
	{
		Piezo::errorTone(5);
		alarmRun = true;
	}
}

// this circuit is called once a microsecond by the motherboard if the 
// safety cutoff is triggered.  
// if cutoff signal is low and has one microsecond has passed
// we expect that a spike in power has occured thus we should reset the
// cutoff circuit
bool Cutoff::noiseResponse(){
	
		// not noise if cutoff has been high for longer than cutoff limit
		if(cutoffCount > CUTOFF_COUNT_LIMIT)
		{
			setAlarm();
			return false;
		}
			
		// if cutoff test line is high do nothing / clear cutoffCount
		if(CUTOFF_TEST.getValue())
		{
			noiseCount = 0;
			cutoffCount++;
		}
		// if cutoff test line is low, reset circuit - check low 5 times
		// before resetting.  
		else{
			if(noiseCount > NOISE_COUNT_LIMIT){
					resetCutoff();
					noiseCount = 0;
			}
			noiseCount++;
			cutoffCount = 0;
		}
		
		return true;
}


