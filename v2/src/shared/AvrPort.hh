#ifndef SHARED_AVR_PORT_HH_
#define SHARED_AVR_PORT_HH_
/*
 * AvrPort.h
 *
 *  Created on: Dec 15, 2009
 *      Author: phooky
 */
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

