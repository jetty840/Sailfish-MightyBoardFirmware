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

#ifndef SHARED_AVR_PORT_HH_
#define SHARED_AVR_PORT_HH_

#include <avr/io.h>

// The AVR port and pin mapping is based on a convention that has held true for all ATMega chips
// released so far: that the ports begin in sequence from register 0x00 from A onwards, and are
// arranged:
// 0 PINx
// 1 DDRx
// 2 PORTx
// This is verified true for the 168/644p/1280.

#define PINx _SFR_MEM8(port_base+0)
#define DDRx _SFR_MEM8(port_base+1)
#define PORTx _SFR_MEM8(port_base+2)

#ifdef __AVR_ATmega1280__
typedef uint16_t port_base_t;
#else
typedef uint8_t port_base_t;
#endif

class Port {
private:
	port_base_t port_base;
public:
	Port() : port_base(0) {}
	Port(port_base_t port_base_in) : port_base(port_base_in) {}

	void setPinDirection(uint8_t pin_index, bool out) {
		DDRx = (DDRx & ~_BV(pin_index)) | (out?_BV(pin_index):0);
	}
	bool getPin(uint8_t pin_index) {
		return (PINx & _BV(pin_index)) != 0;
	}
	void setPin(uint8_t pin_index, bool on) {
		PORTx = (PORTx & ~_BV(pin_index)) | (on?_BV(pin_index):0);
	}
};

extern Port PortA, PortB, PortC, PortD;
#ifdef __AVR_ATmega1280__
extern Port PortE, PortF, PortG, PortH;
extern Port PortJ, PortK, PortL;
#endif // __AVR_ATmega1280__

class Pin {
private:
	Port port;
	uint8_t pin_index : 4;
public:
	Pin() : port(Port()), pin_index(0) {}
	Pin(Port& port_in, uint8_t pin_index_in) : port(port_in), pin_index(pin_index_in) {}
	void setDirection(bool out) { port.setPinDirection(pin_index,out); }
	bool getValue() { return port.getPin(pin_index); }
	void setValue(bool on) { port.setPin(pin_index,on); }
	const uint8_t getPinIndex() const { return pin_index; }
};

#endif // SHARED_AVR_PORT_HH_

