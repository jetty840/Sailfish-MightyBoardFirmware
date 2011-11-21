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

#include <stdint.h>

#if defined (__AVR_ATmega168__) \
    || defined (__AVR_ATmega328__) \
    || defined (__AVR_ATmega644P__)

    typedef uint8_t port_base_t;
    #define NULL_PORT 0xff

#elif defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)

    typedef uint16_t port_base_t;
    #define NULL_PORT 0xffff

#endif


/// The port module represents an eight bit, digital IO port on the
/// AVR microcontroller. This library creates static
///
/// Porting notes:
/// Be sure to define all of the ports supported by your processor, and to
/// verify that the port registers follow the same convention as the 168.
/// \ingroup HardwareLibraries
class AvrPort {
private:
	port_base_t port_base;
public:
        AvrPort();
        AvrPort(port_base_t port_base_in);
        bool isNull();
        void setPinDirection(uint8_t pin_index, bool out);
        bool getPin(uint8_t pin_index);
        void setPin(uint8_t pin_index, bool on);
};


extern AvrPort PortA, PortB, PortC, PortD;

#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
    extern AvrPort PortE, PortF, PortG, PortH;
    extern AvrPort PortJ, PortK, PortL;
#endif // __AVR_ATmega1280__


#endif // SHARED_AVR_PORT_HH_

