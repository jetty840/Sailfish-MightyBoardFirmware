#ifndef PIN_HH
#define PIN_HH

#include "AvrPort.hh"
#include "Pin.hh"

/// \ingroup HardwareLibraries
class Pin {
private:
        AvrPort port;
        uint8_t pin_index : 4;
public:
        Pin() : port(AvrPort()), pin_index(0) {}
        Pin(AvrPort& port_in, uint8_t pin_index_in) : port(port_in), pin_index(pin_index_in) {}
        bool isNull() { return port.isNull(); }
        void setDirection(bool out) { port.setPinDirection(pin_index,out); }
        bool getValue() { return port.getPin(pin_index); }
        void setValue(bool on) { port.setPin(pin_index,on); }
        const uint8_t getPinIndex() const { return pin_index; }
};

#endif // PIN_HH
