#include "AnalogPin.hh"
#include "DebugPin.hh"
#include <avr/io.h>
#include <avr/interrupt.h>

void initAnalogPins(uint8_t bitmask) {
	DDRC &= ~bitmask;
	PORTC &= ~bitmask;

	// enable a2d conversions, interrupt on completion
	ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) |
			_BV(ADEN) | _BV(ADIE);

}

volatile uint16_t* adc_destination;

// We are using the AVcc as our reference.  There's a 100nF cap
// to ground on the AREF pin.
const uint8_t ANALOG_REF = 0x01;

void startAnalogRead(uint8_t pin, volatile uint16_t* destination) {
	// ADSC is cleared when the conversion finishes.
	// We should not start a new read while an existing one is in progress.
	if ( (ADCSRA & _BV(ADSC)) != 0) {
		setDebugLED(false);
	}

	// TODO: ATOMIC
	adc_destination = destination;

	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
	ADMUX = (ANALOG_REF << 6) | (pin & 0x0f);

	// start the conversion.
	ADCSRA |= _BV(ADSC);

	// An interrupt will signal conversion completion.
}

ISR(ADC_vect)
{
	uint8_t low_byte, high_byte;
	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low_byte = ADCL;
	high_byte = ADCH;

	// combine the two bytes
	*adc_destination = (high_byte << 8) | low_byte;
}

