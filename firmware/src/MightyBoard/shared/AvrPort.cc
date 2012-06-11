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

#if defined(__AVR_ATmega644P__) || \
defined(__AVR_ATmega1280__) || \
	defined(__AVR_ATmega2560__)
extern const AvrPort PortA(0x20);
#endif // __AVR_ATmega644P__
extern const AvrPort PortB(0x23);
extern const AvrPort PortC(0x26);
extern const AvrPort PortD(0x29);
#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
extern const AvrPort PortE(0x2C);
extern const AvrPort PortF(0x2F);
extern const AvrPort PortG(0x32);
extern const AvrPort PortH(0x100);
extern const AvrPort PortJ(0x103);
extern const AvrPort PortK(0x106);
extern const AvrPort PortL(0x109);
#endif //__AVR_ATmega1280__

extern const AvrPort NullPort = AvrPort();

AvrPort::AvrPort() : port_base(NULL_PORT) {};

AvrPort::AvrPort(port_base_t port_base_in) : port_base(port_base_in) {
};

bool AvrPort::isNull() const {
	return port_base == NULL_PORT;
};

void AvrPort::setPinDirectionOut(uint8_t pin_mask) const {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		DDRx |= (uint8_t)pin_mask;
	}
};

void AvrPort::setPinDirectionIn(uint8_t pin_mask_inverted) const {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		DDRx &= (uint8_t)pin_mask_inverted;
	}
};

bool AvrPort::getPin(uint8_t pin_mask) const {
	return (uint8_t)((uint8_t)PINx & (uint8_t)pin_mask) != 0;
};

void AvrPort::setPinOn(uint8_t pin_mask) const {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { 
		PORTx |= (uint8_t)pin_mask;
	}
};

void AvrPort::setPinOff(uint8_t pin_mask_inverted) const {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		PORTx &= (uint8_t)pin_mask_inverted;
	}
};
