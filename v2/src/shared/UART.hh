/*
 * UART.hh
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#ifndef MB_UTIL_UART_HH_
#define MB_UTIL_UART_HH_

#include "Packet.hh"
#include <stdint.h>

/**
 * UARTs, when constructed, start off disabled.
 * They begin receiving data only after an enable(true)
 * call is made.  beginSend() calls will send completed
 * packets.
 *
 * All MB UARTs are presumed to run at 38400bps.
 */
class UART {
private:
	const uint8_t index_;
	volatile bool enabled_;
public:
	UART(uint8_t index);
	InPacket in_;
	OutPacket out_;
	void beginSend();
	void enable(bool enabled);
};

enum {
	HOST_UART,
	SLAVE_UART
};

extern UART uart[];

#endif // MB_UTIL_UART_HH_
