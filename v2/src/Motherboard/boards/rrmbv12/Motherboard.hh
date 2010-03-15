/*
 * Motherboard.hh
 *
 *  Created on: Mar 10, 2010
 *      Author: phooky
 */

#ifndef MOTHERBOARD_HH_
#define MOTHERBOARD_HH_

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

#endif /* MOTHERBOARD_HH_ */
