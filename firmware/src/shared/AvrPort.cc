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


#include "AvrPort.hh"

#include <avr/io.h>

// The AVR port and pin mapping is based on a convention that has held true for all ATMega chips
// released so far: that the ports begin in sequence from register 0x00 from A onwards, and are
// arranged:
// 0 PINx
// 1 DDRx
// 2 PORTx
// This is verified true for the 168/328/644p/1280/2560.

// We support three platforms: Atmega168 (1 UART), Atmega644, and Atmega1280/2560
#if defined (__AVR_ATmega168__)     \
    || defined (__AVR_ATmega328__)  \
    || defined (__AVR_ATmega644P__) \
    || defined (__AVR_ATmega1280__) \
    || defined (__AVR_ATmega2560__)
#else
    #error UART not implemented on this processor type!
#endif


#define PINx _SFR_MEM8(port_base+0)
#define DDRx _SFR_MEM8(port_base+1)
#define PORTx _SFR_MEM8(port_base+2)


AvrPort::AvrPort() :
    port_base(NULL_PORT) {
}


AvrPort::AvrPort(port_base_t port_base_in) :
    port_base(port_base_in) {
}

bool AvrPort::isNull() {
    return port_base == NULL_PORT;
}


void AvrPort::setPinDirection(uint8_t pin_index, bool out) {
        DDRx = (DDRx & ~_BV(pin_index)) | (out?_BV(pin_index):0);
}


bool AvrPort::getPin(uint8_t pin_index) {
        return (PINx & _BV(pin_index)) != 0;
}


void AvrPort::setPin(uint8_t pin_index, bool on) {
        PORTx = (PORTx & ~_BV(pin_index)) | (on?_BV(pin_index):0);
}



#if defined(__AVR_ATmega644P__) || \
	defined(__AVR_ATmega1280__) || \
	defined(__AVR_ATmega2560__)
AvrPort PortA(0x20);
#endif // __AVR_ATmega644P__
AvrPort PortB(0x23);
AvrPort PortC(0x26);
AvrPort PortD(0x29);
#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
AvrPort PortE(0x2C);
AvrPort PortF(0x2F);
AvrPort PortG(0x32);
AvrPort PortH(0x100);
AvrPort PortJ(0x103);
AvrPort PortK(0x106);
AvrPort PortL(0x109);
#endif //__AVR_ATmega1280__
