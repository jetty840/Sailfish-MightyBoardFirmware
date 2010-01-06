#include "PSU.hh"
#include <avr/io.h>
#include <util/delay.h>

#define PSU_PORT PORTD
#define PSU_DDR  DDRD
#define PSU_PIN  6

void initPsu() {
	PSU_DDR |= _BV(PSU_PIN);
	turnPsuOn(true);
}

void turnPsuOn(bool on) {
	if (on) {
		PSU_PORT &= ~_BV(PSU_PIN);
		_delay_ms(2000);
	} else {
		PSU_PORT |= _BV(PSU_PIN);
	}
}
