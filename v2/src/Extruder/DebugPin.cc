#include "DebugPin.hh"
#include <avr/io.h>

#define DEBUG_PIN 5

void setDebugLED(bool on) {
	DDRB |= _BV(DEBUG_PIN);
	if (on) {
		PORTB |= _BV(DEBUG_PIN);
	} else {
		PORTB &= ~_BV(DEBUG_PIN);
	}
}
