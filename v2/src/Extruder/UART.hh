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

#ifndef BOARDS_ECV22_UART_HH_
#define BOARDS_ECV22_UART_HH_

#include "Packet.hh"
#include <stdint.h>











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


public:
        static UART& getHostUART() { return hostUART; }


private:
        UART();


        volatile bool enabled_;

public:
	InPacket in;
	OutPacket out;
	void beginSend();
        void enable(bool enabled_);

	// Reset the UART to a listening state.  This is important for
	// RS485-based comms.
	void reset();
};

#endif // BOARDS_ECV22_UART_HH_
