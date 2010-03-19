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

#include "Tool.hh"
#include "Timeout.hh"
#include "UART.hh"
#include "Errors.hh"
#include "Motherboard.hh"

namespace tool {

InPacket& getInPacket() {
	return uart[1].in_;
}

OutPacket& getOutPacket() {
	return uart[1].out_;
}

#define TOOL_PACKET_TIMEOUT_MS 20
#define TOOL_PACKET_TIMEOUT_MICROS (1000*TOOL_PACKET_TIMEOUT_MS)

bool transaction_active = false;
bool locked = false;

Timeout timeout;

/// The tool is considered locked if a transaction is in progress or
/// if the lock was never released.
bool getLock() {
	if (transaction_active || locked)
		return false;
	locked = true;
	return true;
}

void releaseLock() {
	locked = false;
}

void startTransaction() {
	transaction_active = true;
	timeout.start(50000); // 50 ms timeout
	uart[1].in_.reset();
	uart[1].beginSend();
}

bool isTransactionDone() {
	return !transaction_active;
}

void runToolSlice() {
	if (transaction_active) {
		if (uart[1].in_.isFinished())
		{
			transaction_active = false;
		} else if (uart[1].in_.hasError()) {
			transaction_active = false;
			Motherboard::getBoard().indicateError(ERR_SLAVE_PACKET_MISC);
		} else if (timeout.hasElapsed()) {
			uart[1].in_.timeout();
			transaction_active = false;
			Motherboard::getBoard().indicateError(ERR_SLAVE_PACKET_TIMEOUT);
		}
	}
}

}
