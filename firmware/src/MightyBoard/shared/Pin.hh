#ifndef PIN_HH
#define PIN_HH

#include "AvrPort.hh"
#include "Pin.hh"

/// \ingroup HardwareLibraries
class Pin {
private:
	port_base_t port_base;
	bool is_null;
	uint8_t pin_index;
	uint8_t pin_mask;
	uint8_t pin_mask_inverted;

public:
	Pin();
	Pin(const AvrPort& port_in, uint8_t pin_index_in);
	Pin(const Pin& other_pin);
	bool isNull() const;
	void setDirection(bool out) const;
	bool /*Pin::*/getValue() const {
		if (is_null)
			return false; // null pin is always low ... ?
		return (uint8_t)((uint8_t)PINx & (uint8_t)pin_mask) != 0;
	};

	void /*Pin::*/setValue(bool on) const {
		// if (is_null)
		// 	return;
		// uint8_t oldSREG = SREG;
		// cli();
		if (on) {
			PORTx |= pin_mask;
		} else {
			PORTx &= pin_mask_inverted;
		}
		// SREG = oldSREG;
	};

	void /*Pin::*/setValueOn() const {
		// if (is_null)
		// 	return;
		// uint8_t oldSREG = SREG;
		// cli();
		PORTx |= pin_mask;
		// SREG = oldSREG;
	};

	void /*Pin::*/setValueOff() const {
		// if (is_null)
		// 	return;
		// uint8_t oldSREG = SREG;
		// cli();
		PORTx &= pin_mask_inverted;
		// SREG = oldSREG;
	};
	// currently not used:
	//const uint8_t getPinIndex() const { return pin_index; }
};

static const Pin NullPin(NullPort, 0);

#endif // PIN_HH
