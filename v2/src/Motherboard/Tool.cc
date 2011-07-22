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

#define TOOL_PACKET_TIMEOUT_MS 50L
#define TOOL_PACKET_TIMEOUT_MICROS (1000L*TOOL_PACKET_TIMEOUT_MS)

#define DELAY_BETWEEN_TRANSMISSIONS_MICROS (500L)

namespace tool {

bool transaction_active = false;
bool locked = false;
uint8_t retries = RETRIES;

Timeout timeout;

uint8_t tool_index = 0;

uint32_t sent_packet_count;
uint32_t packet_failure_count;
uint32_t packet_retry_count;
uint32_t noise_byte_count;

InPacket& getInPacket() {
        return UART::getSlaveUART().in;
}

OutPacket& getOutPacket() {
        return UART::getSlaveUART().out;
}


// Get the total number of packets that were attempted to be sent to a tool
uint32_t getSentPacketCount() {
    return sent_packet_count;
}

// Get the total number of packets that failed to get a response from a tool
uint32_t getPacketFailureCount() {
    return packet_failure_count;
}

// Get the total packet retries attempted
uint32_t getRetryCount() {
    return packet_retry_count;
}

// Get the total number of received bytes that were discarded as noise
uint32_t getNoiseByteCount() {
    return noise_byte_count;
}


bool getToolVersion() {
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
    out.append8(0); // Index o
    out.append8(SLAVE_CMD_VERSION);
    out.append8(0);  // Technically, we should report our version here, however
    out.append8(0);  // it doesn't actually matter.

    startTransaction();
    // override standard timeout
    timeout.start(TOOL_PACKET_TIMEOUT_MICROS*2);
    releaseLock();
    // WHILE: bounded by tool timeout
    while (!isTransactionDone()) {
            runToolSlice(); // This will most likely time out if there's multiple toolheads.
    }

    if (in.getErrorCode() == PacketError::PACKET_TIMEOUT) {
            return false;
    } else {
            // TODO: Should we actually read the tool version?
//            in.read8(1-2);
    }

    // Check that the extruder was able to process the request
    if (in.read8(0) != 1) {
            return false;
    }

    return true;
}

void setToolIndicatorLED() {
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
    out.append8(0);
    out.append8(SLAVE_CMD_LIGHT_INDICATOR_LED);
    startTransaction();
    // override standard timeout
    timeout.start(TOOL_PACKET_TIMEOUT_MICROS*2);
    releaseLock();
    // WHILE: bounded by tool timeout
    while (!isTransactionDone()) {
            runToolSlice(); // This will most likely time out if there's multiple toolheads.
    }
}

bool reset() {
//	// This code is very lightly modified from handleToolQuery in Host.cc.
//	// We don't give up if we fail to get a lock; we force it instead.
//	Timeout acquire_lock_timeout;
//	acquire_lock_timeout.start(TOOL_PACKET_TIMEOUT_MICROS*2);
//	while (!tool::getLock()) {
//		if (acquire_lock_timeout.hasElapsed()) {
//			locked = true; // grant ourselves the lock
//			transaction_active = false; // abort transaction!
//                        Motherboard::getBoard().indicateError(ERR_SLAVE_LOCK_TIMEOUT);
//			break;
//		}
//	}
//	OutPacket& out = getOutPacket();
//	InPacket& in = getInPacket();
//	out.reset();
//	out.append8(255); // Reset all tools
//	out.append8(SLAVE_CMD_INIT);
//	startTransaction();
//	// override standard timeout
//	timeout.start(TOOL_PACKET_TIMEOUT_MICROS*2);
//	releaseLock();
//	// WHILE: bounded by tool timeout
//	while (!isTransactionDone()) {
//		runToolSlice(); // This will most likely time out if there's multiple toolheads.
//	}

        // Reset packet statistics
        sent_packet_count = 0;
        packet_failure_count = 0;
        packet_retry_count = 0;
        noise_byte_count = 0;

        // Now, test comms by pinging the extruder controller relentlessly.
        // TODO: handle cases where a toolhead is not attached?
        uint8_t i = 0;
        bool result = true;
        while (i < 255 && result) {

            result = getToolVersion();
            i++;
        }
        if (result) {
            setToolIndicatorLED();
        }

        return UART::getSlaveUART().in.isFinished();
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
        sent_packet_count++;

        // Enforce a minimum off-time between transactions
        // TODO: Base this on the time from the last transaction.
        Timeout t;
        t.start(DELAY_BETWEEN_TRANSMISSIONS_MICROS); // wait for xxx us
        while (!t.hasElapsed());

	transaction_active = true;
	timeout.start(TOOL_PACKET_TIMEOUT_MICROS); // 50 ms timeout
	retries = RETRIES;
        UART::getSlaveUART().in.reset();
        UART::getSlaveUART().beginSend();
}

bool isTransactionDone() {
	return !transaction_active;
}

void runToolSlice() {
        UART& uart = UART::getSlaveUART();
	if (transaction_active) {
		if (uart.in.isFinished())
		{
			transaction_active = false;
		} else if (uart.in.hasError()) {
		  if (uart.in.getErrorCode() == PacketError::NOISE_BYTE) {
                    noise_byte_count++;
		    uart.in.reset();
		  } else
			if (retries) {
                                packet_retry_count++;
				retries--;
				timeout.start(TOOL_PACKET_TIMEOUT_MICROS); // 50 ms timeout
				uart.out.prepareForResend();
				uart.in.reset();
				uart.reset();
				uart.beginSend();
			} else {
                                packet_failure_count++;
				transaction_active = false;
                                Motherboard::getBoard().indicateError(ERR_SLAVE_PACKET_MISC);
			}
		} else if (timeout.hasElapsed()) {
			if (retries) {
                                packet_retry_count++;
				retries--;
				timeout.start(TOOL_PACKET_TIMEOUT_MICROS); // 50 ms timeout
				uart.out.prepareForResend();
				uart.in.reset();
				uart.reset();
				uart.beginSend();
			} else {
                                packet_failure_count++;
				uart.in.timeout();
				uart.reset();
				transaction_active = false;
                                Motherboard::getBoard().indicateError(ERR_SLAVE_PACKET_TIMEOUT);
			}
		}
	}
}

}
