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

#include "AnalogPin.hh"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
void initAnalogPins(uint8_t bitmask) {
	DDRC &= ~bitmask;
	PORTC &= ~bitmask;

	// enable a2d conversions, interrupt on completion
	ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) |
			_BV(ADEN) | _BV(ADIE);

}

volatile int16_t* adc_destination;

// We are using the AVcc as our reference.  There's a 100nF cap
// to ground on the AREF pin.
const uint8_t ANALOG_REF = 0x01;

bool isAnalogReady() {
	return (ADCSRA & _BV(ADSC)) == 0;
}

bool startAnalogRead(uint8_t pin, volatile int16_t* destination) {

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		// ADSC is cleared when the conversion finishes.
		// We should not start a new read while an existing one is in progress.
		if (!isAnalogReady()) {
			return false;
		}

		adc_destination = destination;

		// set the analog reference (high two bits of ADMUX) and select the
		// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
		// to 0 (the default).
		ADMUX = (ANALOG_REF << 6) | (pin & 0x0f);

		// start the conversion.
		ADCSRA |= _BV(ADSC);
	}

	// An interrupt will signal conversion completion.
	return true;
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

