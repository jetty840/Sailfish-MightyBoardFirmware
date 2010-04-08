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

class Port {
private:
	volatile uint8_t* port_reg;
	volatile uint8_t* pin_reg;
	volatile uint8_t* direction_reg;
public:
	Port() : port_reg(0), pin_reg(0), direction_reg(0) {}
	Port(volatile uint8_t& port_reg_in,
			volatile uint8_t& pin_reg_in,
			volatile uint8_t& direction_reg_in) :
				port_reg(&port_reg_in), pin_reg(&pin_reg_in),
				direction_reg(&direction_reg_in) {}
	void setPinDirection(uint8_t pin_index, bool out) {
		*direction_reg = (*direction_reg & ~_BV(pin_index)) | (out?_BV(pin_index):0);
	}
	bool getPin(uint8_t pin_index) {
		return (*pin_reg & _BV(pin_index)) != 0;
	}
	void setPin(uint8_t pin_index, bool on) {
		*port_reg = (*port_reg & ~_BV(pin_index)) | (on?_BV(pin_index):0);
	}
};

extern Port PortA, PortB, PortC, PortD;

class Pin {
private:
	Port port;
	uint8_t pin_index;
public:
	Pin() : port(Port()), pin_index(0) {}
	Pin(Port& port_in, uint8_t pin_index_in) : port(port_in), pin_index(pin_index_in) {}
	void setDirection(bool out) { port.setPinDirection(pin_index,out); }
	bool getValue() { return port.getPin(pin_index); }
	void setValue(bool on) { port.setPin(pin_index,on); }
	const uint8_t getPinIndex() const { return pin_index; }
};

#endif // SHARED_AVR_PORT_HH_

