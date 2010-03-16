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

#ifndef BOARDS_RRMBV12_MOTHERBOARD_HH_
#define BOARDS_RRMBV12_MOTHERBOARD_HH_

#include "UART.hh"
#include "StepperInterface.hh"
#include "Types.hh"
#include "PSU.hh"

#define STEPPER_COUNT 3

class Motherboard {
private:
	const static int STEPPERS = STEPPER_COUNT;

	UART host_uart, slave_uart;
	StepperInterface stepper[STEPPERS];
	PSU psu;
	volatile micros_t micros;

	Motherboard();

	static Motherboard motherboard;
public:
	/// Reset the motherboard to its initial state.
	/// This only resets the board, and does not send a reset
	/// to any attached toolheads.
	void reset();

	/// Get the UART that communicates with the host.
	UART& getHostUART() { return host_uart; }
	/// Get the UART that communicates with the toolhead.
	UART& getSlaveUART() { return slave_uart; }

	/// Count the number of steppers available on this board.
	const int getStepperCount() const { return STEPPERS; }
	/// Get the stepper interface for the nth stepper.
	StepperInterface& getStepperInterface(int n)
	{
		return stepper[n];
	}

	/// Get the number of microseconds that have passed since
	/// the board was booted.
	micros_t getCurrentMicros();

	/// Get the power supply unit interface.
	PSU& getPSU() { return psu; }

	/// Write an error code to the debug pin.
	void indicateError(int errorCode);

	/// Get the motherboard instance.
	static Motherboard& getBoard() { return motherboard; }

	/// Perform the timer interrupt routine.
	void doInterrupt();
};

#endif // BOARDS_RRMBV12_MOTHERBOARD_HH_
