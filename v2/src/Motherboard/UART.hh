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

#ifndef UART_HH_
#define UART_HH_

#include "Packet.hh"
#include <stdint.h>

// Only motherboards have a slave UART
#if defined (__AVR_ATmega644P__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
#define HAS_SLAVE_UART
#endif

enum communication_mode {
    RS232,
    RS485
};

/**
 * UARTs, when constructed, start off disabled.
 * They begin receiving data only after an enable(true)
 * call is made.  beginSend() calls will send completed
 * packets.
 *
 */
class UART {
private:
    static UART hostUART;

#ifdef HAS_SLAVE_UART
    static UART slaveUART;
#endif

public:
    static UART& getHostUART() { return hostUART; }

#ifdef HAS_SLAVE_UART
    static UART& getSlaveUART() { return slaveUART; }
#endif

private:
        UART(uint8_t index, communication_mode mode);
        const communication_mode mode_;
	const uint8_t index_;
	volatile bool enabled_;

public:
	InPacket in;
	OutPacket out;
	void beginSend();
	void enable(bool enabled);

	// Reset the UART to a listening state.  This is important for
	// RS485-based comms.
        void reset();
};

#endif // UART_HH_
