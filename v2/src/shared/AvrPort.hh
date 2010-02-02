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
	volatile uint8_t& port_reg_;
	volatile uint8_t& pin_reg_;
	volatile uint8_t& direction_reg_;
public:
	Port(volatile uint8_t& port_reg,
			volatile uint8_t& pin_reg,
			volatile uint8_t& direction_reg) :
				port_reg_(port_reg), pin_reg_(pin_reg),
				direction_reg_(direction_reg) {}
	void setPinDirection(uint8_t pin_index, bool out) {
		direction_reg_ = (direction_reg_ & ~_BV(pin_index)) | (out?_BV(pin_index):0);
	}
	bool getPin(uint8_t pin_index) {
		return (pin_reg_ & _BV(pin_index)) != 0;
	}
	void setPin(uint8_t pin_index, bool on) {
		port_reg_ = (port_reg_ & ~_BV(pin_index)) | (on?_BV(pin_index):0);
	}
	uint16_t getAnalogPin(uint8_t pin_index);
};

extern Port PortA, PortB, PortC, PortD;

class Pin {
private:
	Port& port_;
	const uint8_t pin_index_;
public:
	Pin(Port& port, uint8_t pin_index) : port_(port), pin_index_(pin_index) {}
	void setDirection(bool out) { port_.setPinDirection(pin_index_,out); }
	bool getValue() { return port_.getPin(pin_index_); }
	void setValue(bool on) { port_.setPin(pin_index_,on); }
	const uint8_t getPinIndex() const { return pin_index_; }
	// This is a blocking call.  You may want to implement an interrupt-driven version
	// if you're short on cycles.
	uint16_t getAnalogValue() { return port_.getAnalogPin(pin_index_); }
};

#endif // SHARED_AVR_PORT_HH_

