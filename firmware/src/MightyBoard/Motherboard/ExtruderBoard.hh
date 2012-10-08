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

/// Extruder heating element on v34 Extruder controller
/// \ingroup ECv34
class ExtruderHeatingElement : public HeatingElement {
public:
	ExtruderHeatingElement(uint8_t id);
	void setHeatingElement(uint8_t value);
	uint8_t heater_id;
	
};

/// Main class for Extruder controller version 3.4
/// \ingroup ECv34
class ExtruderBoard {
//private:
//        static ExtruderBoard extruder_board;

public:
			/**
			 *
			 * @param slave_id_in
			 * @param HeaterPin_In
			 * @param FanPin_In
			 * @param ThermocouplePin_In
			 * @param eeprom_base Start of the chunk of EEPROM  memorycontaining extruder settings
			 * @return
			 */
			ExtruderBoard(uint8_t slave_id_in, Pin HeaterPin_In, Pin FanPin_In,
					Pin ThermocouplePin_In, uint16_t eeprom_base);
private:
        Thermocouple extruder_thermocouple;
        ExtruderHeatingElement extruder_element;
        Heater extruder_heater;
        
        uint8_t slave_id;

        Pin Heater_Pin;

        CoolingFan coolingFan;
        uint8_t* eeprom_base;

public:
	void reset();

	//void resetEeprom()

	void runExtruderSlice();
        
	void setFan(uint8_t on);

  	Heater& getExtruderHeater() { return extruder_heater; }

	uint8_t getSlaveID() { return slave_id; }
};

#endif // BOARDS_ECV34_EXTRUDER_BOARD_HH_
