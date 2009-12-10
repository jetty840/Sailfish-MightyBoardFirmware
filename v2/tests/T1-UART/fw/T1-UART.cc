/*
 * T1-UART.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "util/UART.hh"
#include "util/DebugPacketProcessor.hh"

int main() {
	while (1) {
		if (uart[0].in_.isFinished()) {
			if (processDebugPacket(uart[0].in_, uart[0].out_)) {
				uart[0].in_.reset();
				uart[0].beginSend();
			} else {
				uart[0].in_.reset();
			}
			while (!uart[0].out_.isFinished())
				;
			uart[0].out_.reset();
		}
	}
	return 0;
}
