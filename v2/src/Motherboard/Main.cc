/*
 * T4-Commands.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "DebugPacketProcessor.hh"
#include "Host.hh"
#include "Tool.hh"
#include "Command.hh"
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "Timeout.hh"
#include "Steppers.hh"
#include "Motherboard.hh"

void reset() {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		steppers::abort();
		command::reset();
		Motherboard::getBoard().reset();
	}
}

int main() {
	steppers::init(Motherboard::getBoard());
	reset();
	sei();
	//steppers.setTarget(points[0],500);
	while (1) {
		// Toolhead interaction thread.
		tool::runToolSlice();
		// Host interaction thread.
		runHostSlice();
		// SD command buffer read/refill thread.
		//runSDSlice();
		// Command handling thread.
		command::runCommandSlice();
	}
	return 0;
}
