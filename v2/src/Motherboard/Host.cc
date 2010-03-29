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

#include "Host.hh"
#include "Command.hh"
#include "Tool.hh"
#include "Commands.hh"
#include "Steppers.hh"
#include "DebugPacketProcessor.hh"
#include "Timeout.hh"
#include "Version.hh"
#include <util/atomic.h>
#include <avr/eeprom.h>
#include "Main.hh"
#include "Errors.hh"
#include "SDCard.hh"

/// Identify a command packet, and process it.  If the packet is a command
/// packet, return true, indicating that the packet has been queued and no
/// other processing needs to be done. Otherwise, processing of this packet
/// should drop through to the next processing level.
bool processCommandPacket(const InPacket& from_host, OutPacket& to_host);
bool processQueryPacket(const InPacket& from_host, OutPacket& to_host);

Timeout packet_in_timeout;

#define HOST_PACKET_TIMEOUT_MS 20
#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

void runHostSlice() {
	InPacket& in = Motherboard::getBoard().getHostUART().in;
	OutPacket& out = Motherboard::getBoard().getHostUART().out;
	if (out.isSending()) {
		// still sending; wait until send is complete before reading new host packets.
		return;
	}
	if (in.isStarted() && !in.isFinished()) {
		if (!packet_in_timeout.isActive()) {
			// initiate timeout
			packet_in_timeout.start(HOST_PACKET_TIMEOUT_MICROS);
		} else if (packet_in_timeout.hasElapsed()) {
			in.timeout();
		}
	}
	if (in.hasError()) {
		// Reset packet quickly and start handling the next packet.
		// Report error code.
		Motherboard::getBoard().indicateError(ERR_HOST_PACKET_TIMEOUT);
		in.reset();
	}
	if (in.isFinished()) {
		packet_in_timeout.abort();
		out.reset();
#if defined(HONOR_DEBUG_PACKETS) && (HONOR_DEBUG_PACKETS == 1)
		if (processDebugPacket(in, out)) {
			// okay, processed
		} else
#endif
		if (processCommandPacket(in, out)) {
			// okay, processed
		} else if (processQueryPacket(in, out)) {
			// okay, processed
		} else {
			// Unrecognized command
			out.append8(RC_CMD_UNSUPPORTED);
		}
		in.reset();
		Motherboard::getBoard().getHostUART().beginSend();
	}
}

/// Identify a command packet, and process it.  If the packet is a command
/// packet, return true, indicating that the packet has been queued and no
/// other processing needs to be done. Otherwise, processing of this packet
/// should drop through to the next processing level.
bool processCommandPacket(const InPacket& from_host, OutPacket& to_host) {
	if (from_host.getLength() >= 1) {
		uint8_t command = from_host.read8(0);
		if ((command & 0x80) != 0) {
			// Queue command, if there's room.
			// Turn off interrupts while querying or manipulating the queue!
			ATOMIC_BLOCK(ATOMIC_FORCEON) {
				const uint8_t command_length = from_host.getLength();
				if (command::getRemainingCapacity() >= command_length) {
					// Append command to buffer
					for (int i = 0; i < command_length; i++) {
						command::push(from_host.read8(i));
					}
					to_host.append8(RC_OK);
				} else {
					to_host.append8(RC_BUFFER_OVERFLOW);
				}
			}
			return true;
		}
	}
	return false;
}

bool processQueryPacket(const InPacket& from_host, OutPacket& to_host) {
	if (from_host.getLength() >= 1) {
		uint8_t command = from_host.read8(0);
		if ((command & 0x80) == 0) {
			// Is query command.
			switch (command) {
			case HOST_CMD_VERSION:
				to_host.append8(RC_OK);
				to_host.append16(firmware_version);
				return true;
			case HOST_CMD_INIT:
			case HOST_CMD_CLEAR_BUFFER: // equivalent at current time
			case HOST_CMD_ABORT: // equivalent at current time
				reset();
				to_host.append8(RC_OK);
				return true;
			case HOST_CMD_GET_BUFFER_SIZE:
				to_host.append8(RC_OK);
				to_host.append32(command::getRemainingCapacity());
				return true;
			case HOST_CMD_GET_POSITION:
				ATOMIC_BLOCK(ATOMIC_FORCEON) {
					const Point p = steppers::getPosition();
					to_host.append8(RC_OK);
					to_host.append32(p[0]);
					to_host.append32(p[1]);
					to_host.append32(p[2]);
					to_host.append8(0); // todo: endstops
				}
				return true;
			case HOST_CMD_PLAYBACK_CAPTURE:
				{
					const int MAX_FILE_LEN = MAX_PACKET_PAYLOAD-1;
					to_host.append8(RC_OK);
					char fnbuf[MAX_FILE_LEN];
					for (int idx = 1; idx < from_host.getLength(); idx++) {
						fnbuf[idx-1] = from_host.read8(idx);
					}
					fnbuf[MAX_FILE_LEN-1] = '\0';
					to_host.append8(sdcard::startPlayback(fnbuf));
				}
				return true;
			case HOST_CMD_NEXT_FILENAME:
				{
					to_host.append8(RC_OK);
					uint8_t resetFlag = from_host.read8(1);
					if (resetFlag != 0) {
						sdcard::SdErrorCode e = sdcard::directoryReset();
						if (e != sdcard::SD_SUCCESS) {
							to_host.append8(e);
							to_host.append8(0);
							return true;
						}
					}
					const int MAX_FILE_LEN = MAX_PACKET_PAYLOAD-1;
					char fnbuf[MAX_FILE_LEN];
					sdcard::SdErrorCode e = sdcard::directoryNextEntry(fnbuf,MAX_FILE_LEN);
					to_host.append8(e);
					uint8_t idx;
					for (idx = 0; (idx < MAX_FILE_LEN) && (fnbuf[idx] != 0); idx++) {
						to_host.append8(fnbuf[idx]);
					}
					to_host.append8(0);
				}
				return true;
			case HOST_CMD_GET_RANGE:
			case HOST_CMD_SET_RANGE:
				break; // not yet implemented
			case HOST_CMD_PAUSE:
				command::pause(!command::isPaused());
				to_host.append8(RC_OK);
				return true;
			case HOST_CMD_TOOL_QUERY: {
				// Quick sanity assert: ensure that host packet length >= 2
				// (Payload must contain toolhead address and at least one byte)
				if (from_host.getLength() < 2) {
					to_host.append8(RC_GENERIC_ERROR);
					Motherboard::getBoard().indicateError(ERR_HOST_TRUNCATED_CMD);
					return true;
				}
				Timeout acquire_lock_timeout;
				acquire_lock_timeout.start(HOST_TOOL_RESPONSE_TIMEOUT_MS);
				while (!tool::getLock()) {
					if (acquire_lock_timeout.hasElapsed()) {
						to_host.append8(RC_DOWNSTREAM_TIMEOUT);
						Motherboard::getBoard().indicateError(ERR_SLAVE_LOCK_TIMEOUT);
						return true;
					}
				}
				OutPacket& out = tool::getOutPacket();
				InPacket& in = tool::getInPacket();
				out.reset();
				for (int i = 1; i < from_host.getLength(); i++) {
					out.append8(from_host.read8(i));
				}
				// Timeouts are handled inside the toolslice code; there's no need
				// to check for timeouts on this loop.
				tool::startTransaction();
				while (!tool::isTransactionDone()) {
					tool::runToolSlice();
				}
				if (in.getErrorCode() == PacketError::PACKET_TIMEOUT) {
					to_host.append8(RC_DOWNSTREAM_TIMEOUT);
				} else {
					// Copy payload back. Start from 0-- we need the response code.
					for (int i = 0; i < in.getLength(); i++) {
						to_host.append8(in.read8(i));
					}
				}
				tool::releaseLock();
			}
				return true;
			case HOST_CMD_IS_FINISHED:
				to_host.append8(RC_OK);
				ATOMIC_BLOCK(ATOMIC_FORCEON) {
					bool done = !steppers::isRunning() && command::isEmpty();
					to_host.append8(done?1:0);
				}
				return true;
			case HOST_CMD_READ_EEPROM: {
				uint16_t offset = from_host.read16(1);
				uint8_t length = from_host.read8(3);
				uint8_t data[16];
				eeprom_read_block(data, (const void*) offset, length);
				to_host.append8(RC_OK);
				for (int i = 0; i < length; i++) {
					to_host.append8(data[i]);
				}
			}
				return true;
			case HOST_CMD_WRITE_EEPROM: {
				uint16_t offset = from_host.read16(1);
				uint8_t length = from_host.read8(3);
				uint8_t data[16];
				eeprom_read_block(data, (const void*) offset, length);
				for (int i = 0; i < length; i++) {
					data[i] = from_host.read8(i + 4);
				}
				eeprom_write_block(data, (void*) offset, length);
				to_host.append8(RC_OK);
				to_host.append8(length);
			}
				return true;
			}
		}
	}
	return false;
}
