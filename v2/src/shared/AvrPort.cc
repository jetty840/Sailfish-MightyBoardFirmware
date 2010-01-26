/*
 * AvrPort.cc
 *
 *  Created on: Dec 22, 2009
 *      Author: phooky
 */

#include "AvrPort.hh"

#ifdef __AVR_ATmega644P__
Port PortA(PORTA, PINA, DDRA);
#endif // __AVR_ATmega644P__
Port PortB(PORTB, PINB, DDRB);
Port PortC(PORTC, PINC, DDRC);
Port PortD(PORTD, PIND, DDRD);

// From Wiring
uint16_t Port::getAnalogPin(uint8_t pin_index) {
	uint8_t low_byte, high_byte;

	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
	ADMUX = (analog_reference << 6) | (pin_index & 0x0f);

#if defined(__AVR_ATmega1280__)
	// the MUX5 bit of ADCSRB selects whether we're reading from channels
	// 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
	ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin_index >> 3) & 0x01) << MUX5);
#endif

	// start the conversion
	sbi(ADCSRA, ADSC);

	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC)) {}

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low_byte = ADCL;
	high_byte = ADCH;

	// combine the two bytes
	return (high << 8) | low;
}
