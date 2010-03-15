/*
 * Motherboard.hh
 *
 *  Created on: Mar 10, 2010
 *      Author: phooky
 */

#ifndef MOTHERBOARD_HH_
#define MOTHERBOARD_HH_

typedef int32_t micros_t;

#include "UART.hh"
#include "StepperInterface.hh"
#include "Timers.hh"
#include "PSU.hh"

class Motherboard {
private:
	const static int STEPPERS = 3;

	UART uart[2];
	StepperInterface stepper[STEPPERS];

	Motherboard();

	static Motherboard motherboard;
public:
	/// Reset the motherboard to its initial state.
	/// This only resets the board, and does not send a reset
	/// to any attached toolheads.
	void reset();

	/// Get the UART that communicates with the host.
	UART& getHostUART() { return uart[0]; }
	/// Get the UART that communicates with the toolhead.
	UART& getSlaveUART() { return uart[1]; }

	/// Count the number of steppers available on this board.
	const int getStepperCount() const { return STEPPERS; }
	/// Get the stepper interface for the nth stepper.
	StepperInterface getStepperInterface(int n)
	{
		return stepper[n];
	}

	/// Get the number of microseconds that have passed since
	/// the board was booted.
	micros_t getCurrentMicros();

	/// Get the power supply unit interface.
	PSU& getPSU();

	/// Write an error code to the debug pin.
	void indicateError(int errorCode);

	/// Get the motherboard instance.
	static Motherboard& getBoard() { return motherboard; }
};

#endif /* MOTHERBOARD_HH_ */
