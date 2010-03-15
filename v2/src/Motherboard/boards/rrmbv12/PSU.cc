#include "PSU.hh"
#include <avr/io.h>
#include <util/delay.h>
#include "Configuration.hh"
#include "AvrPort.hh"

void PSU::init() {
	PSU_PIN.setDirection(true);
	turnOn(true);
}

void PSU::turnOn(bool on) {
	// PSU pin is pulled low to turn on power supply
	PSU_PIN.setValue(!on);
}
