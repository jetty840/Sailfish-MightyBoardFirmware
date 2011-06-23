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

#ifndef BOARDS_ECV22_EXTRUDER_BOARD_HH_
#define BOARDS_ECV22_EXTRUDER_BOARD_HH_

#include "UART.hh"
#include "ExtruderMotor.hh"
#include "Thermistor.hh"
#include "HeatingElement.hh"
#include "Heater.hh"

// Definition of the extruder heating element
class ExtruderHeatingElement : public HeatingElement {
public:
	void setHeatingElement(uint8_t value);
};

// Definition of the heated build platform heating element
class BuildPlatformHeatingElement : public HeatingElement {
public:
	void setHeatingElement(uint8_t value);
};

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
	void setMotorSpeedRPM(uint32_t speed, bool direction);
#ifdef DEFAULT_EXTERNAL_STEPPER
	// Hack to decouple holding torque from RPM speed
	// Stops/starts the motor while holding torque
	void setMotorOn(bool on);
#endif
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

        void flashIndicatorLED() {}

	bool isUsingPlatform() { return using_platform; }
	void setUsingPlatform(bool is_using);
	void setUsingRelays(bool is_using);
	// Index 0 = D9, Index 1 = D10.  Value = -1 to turn off, 0-180 to set position.
	void setServo(uint8_t index, int value);
private:
	Thermistor extruder_thermistor;
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

	uint8_t resetFlags;
};

#endif // BOARDS_ECV22_EXTRUDER_BOARD_HH_
