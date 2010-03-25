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

#ifndef HEATER_H
#define HEATER_H

#include "TemperatureSensor.hh"
#include "HeatingElement.hh"
#include "AvrPort.hh"
#include "PID.hh"
#include "Types.hh"
#include "Timeout.hh"

class Heater
{
  private:
	TemperatureSensor& sensor;
    HeatingElement& element;
    Timeout next_read_timeout;
    
    int current_temperature;

    PID pid;

    const static int UPDATE_INTERVAL_MICROS = 60;

  public:
    micros_t last_update;
    Heater(TemperatureSensor& sensor, HeatingElement& element);
    
    int get_current_temperature(); 
    void set_target_temperature(int temp);
    bool hasReachedTargetTemperature();

    // Call once each temperature interval
    void manage_temperature();

    void set_output(uint8_t value);
};

#endif // HEATER_H
