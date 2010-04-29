/*
 * DebugPacketProcessor.cc
 *
 *  Created on: Dec 9, 2009
 *      Author: phooky
 */

#include "DebugPacketProcessor.hh"
#include "UART.hh"
#include "Timeout.hh"
#include "Configuration.hh"
#if HAS_SLAVE_UART
#include "Motherboard.hh"
#include "Errors.hh"
#include "Tool.hh"
#endif
#if HAS_COMMAND_QUEUE
#include "Command.hh"
#endif // HAS_COMMAND_QUEUE
namespace CommandCode {
enum {
	DEBUG_ECHO = 0x70,
	DEBUG_GENERATE_BAD_PACKET = 0x71,
	DEBUG_SIMULATE_BAD_PACKET = 0x72,
	DEBUG_SLAVE_PASSTHRU = 0x73,
	DEBUG_CLEAR_COMMAND_QUEUE = 0x74,
	DEBUG_COMMAND_QUEUE_FILLER = 0xF0
};
}

namespace BadReceptionCode {
enum {
	RECEIVED_NOISE = 1,
	BAD_PACKET_LENGTH = 2,
	BAD_CRC = 3,
	TIMEOUT = 4
};
}

namespace BadResponseCode {
enum {
	NO_RESPONSE = 1,
	SKIP_START_BYTE = 2,
	BAD_PACKET_LENGTH = 3,
	BAD_CRC = 4,
	TIMEOUT = 5
};

}

/// Identify a debug packet, and process it.  If the packet is a debug
/// packet, return true, indicating that no further processing should
/// be done.  Otherwise, processing of this packet should drop through
/// to the next processing level.
bool processDebugPacket(const InPacket& from_host, OutPacket& to_host) {
	if (from_host.getLength() == 0) {
		return false;
	} // drop through on a nop packet
	uint8_t command = from_host.read8(0);

	if ((command & 0x70) == 0x70) {
		// This is a debug packet
		if (command == CommandCode::DEBUG_ECHO) {
			// We start from 1 so we can skip the debug command byte.
			to_host.reset();
			for (int i = 1; i < from_host.getLength(); i++) {
				to_host.append8(from_host.read8(i));
			}
			return true;
		} else if (command == CommandCode::DEBUG_GENERATE_BAD_PACKET) {
			// TODO
		} else if (command == CommandCode::DEBUG_SIMULATE_BAD_PACKET) {
			// TODO
		} else if (command == CommandCode::DEBUG_SLAVE_PASSTHRU) {
			return true;
		}
		return false;
	} else {
		// This is not a debug packet
		return false;
	}
}
