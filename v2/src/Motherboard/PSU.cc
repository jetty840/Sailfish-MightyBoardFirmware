#include "PSU.hh"
#include <avr/io.h>
#include <util/delay.h>
#include "Configuration.hh"
#include "AvrPort.hh"

namespace psu {

void init() {
	PSU_PIN.setDirection(true);
	turnOn(true);
}

void turnOn(bool on) {
	// PSU pin is pulled low to turn on power supply
	PSU_PIN.setValue(!on);
}

}
