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

#include "UART.hh"
#include "ExtruderMotor.hh"
#include "Thermistor.hh"
#include "Thermocouple.hh"
#include "HeatingElement.hh"
#include "Heater.hh"
#include "SoftwareServo.hh"
#include "EepromMap.hh"
#include "CoolingFan.hh"

/// \defgroup ECv34
/// Code specific to the Extruder Controller version 3.4 (gen4 hardware)

/// Extruder heating element on v34 Extruder controller
/// \ingroup ECv34
class ExtruderHeatingElement : public HeatingElement {
public:
	void setHeatingElement(uint8_t value);
};


/// Build platform heating element on v34 Extruder controller
/// \ingroup ECv34
class BuildPlatformHeatingElement : public HeatingElement {
public:
	void setHeatingElement(uint8_t value);
};

/// Main class for Extruder controller version 3.4
/// \ingroup ECv34
class ExtruderBoard {
public:
	void reset(uint8_t resetFlags);
	// Return the processor's reset status flags.  These are useful
	// for diagnosing what might have triggered the last processor
	// reset.
	uint8_t getResetFlags();

	Heater& getExtruderHeater() { return extruder_heater; }
	Heater& getPlatformHeater() { return platform_heater; }

	void setMotorSpeed(int16_t speed);
	void setMotorSpeedRPM(uint32_t speed, bool direction) {} // Unsupported on 3.4
	void setFan(bool on);
	void setValve(bool on);
	UART& getHostUART() { return UART::getHostUART(); }
	static ExtruderBoard& getBoard() { return extruder_board; }
	/// Get the number of microseconds that have passed since
	/// the board was initialized.  This value will wrap after
	/// 2**16 microseconds; callers should compensate for this.
	micros_t getCurrentMicros();
	/// Perform the timer interrupt routine.
	void doInterrupt();
	/// Indicate an error by manipulating the debug LED.
	void indicateError(int errorCode);

        void lightIndicatorLED();
	bool isUsingPlatform() { return using_platform; }
	void setUsingPlatform(bool is_using);

	// Index 0 = ServoA, Index 1 = ServoB.  Value = -1 to turn off, 0-180 to set position.
        void setServo(uint8_t index, int value);

        uint8_t getSlaveID() { return slave_id; }
private:
	Thermocouple extruder_thermocouple;
	Thermistor platform_thermistor;
	ExtruderHeatingElement extruder_element;
	BuildPlatformHeatingElement platform_element;
	Heater extruder_heater;
	Heater platform_heater;
	bool using_platform;
	/// Microseconds since board initialization
	volatile micros_t micros;
	ExtruderBoard();
	static ExtruderBoard extruder_board;

	SoftwareServo servoA;
	SoftwareServo servoB;
	CoolingFan coolingFan;

	uint8_t resetFlags;

        uint8_t slave_id;
};

#endif // BOARDS_ECV34_EXTRUDER_BOARD_HH_
