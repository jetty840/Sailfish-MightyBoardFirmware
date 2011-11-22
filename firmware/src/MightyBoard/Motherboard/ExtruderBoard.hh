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

#ifndef BOARDS_ECV34_EXTRUDER_BOARD_HH_
#define BOARDS_ECV34_EXTRUDER_BOARD_HH_

//#include "UART.hh"
//#include "ExtruderMotor.hh"
//#include "Thermistor.hh"
#include "Thermocouple.hh"
#include "HeatingElement.hh"
#include "Heater.hh"
//#include "SoftwareServo.hh"
#include "EepromMap.hh"
#include "CoolingFan.hh"
//#include "MotorController.hh"

/// \defgroup ECv34
/// Code specific to the Extruder Controller version 3.4 (gen4 hardware)

/// Main class for Extruder controller version 3.4
/// \ingroup ECv34
class ExtruderBoard {
//private:
//        static ExtruderBoard extruder_board;

public:
//        static ExtruderBoard& getBoard() { return extruder_board; }
			ExtruderBoard(uint8_t slave_id_in, Pin HeaterPin_In, Pin FanPin_In, Pin ThermocouplePin_In);
private:
        Thermocouple extruder_thermocouple;
        ExtruderHeatingElement extruder_element;
        Heater extruder_heater;
        
        Pin Heater_Pin;
        Pin Fan_Pin;

        uint8_t slave_id;

        CoolingFan coolingFan;

public:
	void reset();

        void runExtruderSlice();

  	Heater& getExtruderHeater() { return extruder_heater; }

	void setFan(bool on);

	uint8_t getSlaveID() { return slave_id; }
};

#endif // BOARDS_ECV34_EXTRUDER_BOARD_HH_
