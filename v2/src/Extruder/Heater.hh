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
    Timeout next_pid_timeout;
    Timeout next_sense_timeout;
    micros_t sample_interval_micros;
    
    int current_temperature;
    uint16_t eeprom_base;

    PID pid;
    bool bypassing_PID;

    bool fail_state;

    // This is the interval between PID calculations.  Longer updates are (counterintuitively)
    // better since we're using discrete D.
    const static micros_t UPDATE_INTERVAL_MICROS = 500L * 1000L;

    void fail();

  public:
    Heater(TemperatureSensor& sensor, HeatingElement& element, const micros_t sample_interval_micros, const uint16_t eeprom_base);
    
    int get_current_temperature();
    int get_set_temperature();
    void set_target_temperature(int temp);
    bool has_reached_target_temperature();
    bool has_failed();

    // Call once each temperature interval
    void manage_temperature();

    void set_output(uint8_t value);

    // Reset to board-on state
    void reset();

    int getPIDErrorTerm();
    int getPIDDeltaTerm();
    int getPIDLastOutput();
};

#endif // HEATER_H
