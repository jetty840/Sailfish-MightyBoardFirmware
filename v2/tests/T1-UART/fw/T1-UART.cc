/*
 * T1-UART.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "util/UART.hh"
#include "util/DebugPacketProcessor.hh"

int main()
{
	while(1) {
		if (processDebugPacket(uart[0].in_, uart[0].out_))
		{
			uart[0].beginSend();
		}
	}
	return 0;
}
