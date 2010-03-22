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
#include "ExtruderHeatingElement.hh"
#include "Heater.hh"

extern UART uart[];

class ExtruderBoard {
public:
	void reset();

	Heater& getExtruderHeater() { return extruder_heater; }

	void setMotorSpeed(int16_t speed);
	UART& getHostUART() { return uart[0]; }
	static ExtruderBoard& getBoard() { return extruderBoard; }
	/// Get the number of microseconds that have passed since
	/// the board was initialized.  This value will wrap after
	/// 2**16 microseconds; callers should compensate for this.
	micros_t getCurrentMicros();
	/// Perform the timer interrupt routine.
	void doInterrupt();
private:
	Thermistor extruder_thermistor;
	ExtruderHeatingElement extruder_element;
	Heater extruder_heater;
	/// Microseconds since board initialization
	volatile micros_t micros;
	ExtruderBoard();
	static ExtruderBoard extruderBoard;
};

#endif // BOARDS_ECV22_EXTRUDER_BOARD_HH_
