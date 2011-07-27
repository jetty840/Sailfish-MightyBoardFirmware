#include "SoftwareServo.hh"

SoftwareServo::SoftwareServo(Pin pin) :
	pin(pin)
{
	pin.setDirection(true);
	pin.setValue(false);
}

void SoftwareServo::setPosition(uint8_t position) {
	// Program the timer match value so that we generate a pulse width per:
	//  http://www.servocity.com/html/hs-311_standard.html
	//  600us + (value * 10)
	//  so 0deg = 600us, 90deg = 1500us, 180deg = 2400us
	if (position > 180) {
		position = 180;
	}

	counts = 600 + 10*position;
}

void SoftwareServo::enable() {
	enabled = true;
}

void SoftwareServo::disable() {
	enabled = false;
	pin.setValue(false);
}
