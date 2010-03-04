/*
 * TemperatureThread.cc
 *
 *  Created on: Mar 1, 2010
 *      Author: phooky
 */
#include "TemperatureThread.hh"
#include "Heater.hh"

void runTempSlice() {
	extruder_heater.manage_temperature();
}
