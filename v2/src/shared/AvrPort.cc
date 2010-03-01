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
