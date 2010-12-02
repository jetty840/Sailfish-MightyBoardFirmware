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
#include "Errors.hh"
#include "Motherboard.hh"
#include "Commands.hh"

#define RETRIES 5
namespace tool {

InPacket& getInPacket() {
	return Motherboard::getBoard().getSlaveUART().in;
}

OutPacket& getOutPacket() {
	return Motherboard::getBoard().getSlaveUART().out;
}

#define TOOL_PACKET_TIMEOUT_MS 50L
#define TOOL_PACKET_TIMEOUT_MICROS (1000L*TOOL_PACKET_TIMEOUT_MS)

bool transaction_active = false;
bool locked = false;
uint8_t retries = RETRIES;

Timeout timeout;

uint8_t tool_index = 0;

bool reset() {
	// This code is very lightly modified from handleToolQuery in Host.cc.
	// We don't give up if we fail to get a lock; we force it instead.
	Timeout acquire_lock_timeout;
	acquire_lock_timeout.start(TOOL_PACKET_TIMEOUT_MICROS*2);
	while (!tool::getLock()) {
		if (acquire_lock_timeout.hasElapsed()) {
			locked = true; // grant ourselves the lock
			transaction_active = false; // abort transaction!
			Motherboard::getBoard().indicateError(ERR_SLAVE_LOCK_TIMEOUT);
			break;
		}
	}
	OutPacket& out = getOutPacket();
	InPacket& in = getInPacket();
	out.reset();
	out.append8(255); // Reset all tools
	out.append8(SLAVE_CMD_INIT);
	startTransaction();
	// override standard timeout
	timeout.start(TOOL_PACKET_TIMEOUT_MICROS*2);
	releaseLock();
	// WHILE: bounded by tool timeout
	while (!isTransactionDone()) {
		runToolSlice(); // This will most likely time out if there's multiple toolheads.
	}
	return Motherboard::getBoard().getSlaveUART().in.isFinished();
}

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
	timeout.start(TOOL_PACKET_TIMEOUT_MICROS); // 50 ms timeout
	retries = RETRIES;
	Motherboard::getBoard().getSlaveUART().in.reset();
	Motherboard::getBoard().getSlaveUART().beginSend();
}

bool isTransactionDone() {
	return !transaction_active;
}

void runToolSlice() {
	UART& uart = Motherboard::getBoard().getSlaveUART();
	if (transaction_active) {
		if (uart.in.isFinished())
		{
			transaction_active = false;
		} else if (uart.in.hasError()) {
			if (retries) {
				retries--;
				timeout.start(TOOL_PACKET_TIMEOUT_MICROS); // 50 ms timeout
				uart.out.prepareForResend();
				uart.in.reset();
				uart.reset();
				uart.beginSend();
			} else {
				transaction_active = false;
				Motherboard::getBoard().indicateError(ERR_SLAVE_PACKET_MISC);
			}
		} else if (timeout.hasElapsed()) {
			if (retries) {
				retries--;
				timeout.start(TOOL_PACKET_TIMEOUT_MICROS); // 50 ms timeout
				uart.out.prepareForResend();
				uart.in.reset();
				uart.reset();
				uart.beginSend();
			} else {
				uart.in.timeout();
				uart.reset();
				transaction_active = false;
				Motherboard::getBoard().indicateError(ERR_SLAVE_PACKET_TIMEOUT);
			}
		}
	}
}

}
