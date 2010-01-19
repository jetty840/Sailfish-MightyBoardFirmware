#include "HostThread.hh"
#include "UART.hh"
#include "CommandQueue.hh"
#include "QueryPacketProcessor.hh"
#include "DebugPacketProcessor.hh"
#include "Timeout.hh"
#include <util/atomic.h>
#include "DebugPin.hh"

/// Identify a command packet, and process it.  If the packet is a command
/// packet, return true, indicating that the packet has been queued and no
/// other processing needs to be done. Otherwise, processing of this packet
/// should drop through to the next processing level.
bool processCommandPacket(const InPacket& from_host, OutPacket& to_host);

Timeout packet_in_timeout;

#define HOST_PACKET_TIMEOUT_MS 20
#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

Timeout tool_response_timeout;

#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

bool waiting_for_tool;

void runHostSlice() {
	InPacket& in = uart[0].in_;
	OutPacket& out = uart[0].out_;
	if (out.isSending()) {
		// still sending; wait until send is complete before reading new host packets.
		return;
	}
	if (in.isStarted() && !in.isFinished()) {
		if (!packet_in_timeout.isActive()) {
			// initiate timeout
			packet_in_timeout = Timeout(HOST_PACKET_TIMEOUT_MICROS);
		} else if (packet_in_timeout.hasElapsed()) {
			in.timeout();
		}
	}
	if (in.hasError()) {
		// REPORTING: report error.
		// Reset packet quickly and start handling the next packet.
		//if (in.getErrorCode() == PacketError::NOISE_BYTE) setDebugLED(false);
		in.reset();
	}
	if (in.isFinished()) {
		packet_in_timeout.abort();
		out.reset();
		if (processDebugPacket(in, out)) {
			// okay, processed
		} else if (processCommandPacket(in, out)) {
			// okay, processed
		} else if (processQueryPacket(in, out)) {
			// okay, processed
		} else {
			// Unrecognized command
			out.append8(RC_CMD_UNSUPPORTED);
		}
		in.reset();
		uart[0].beginSend();
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
				if (command_buffer.getRemainingCapacity() >= command_length) {
					// Append command to buffer
					for (int i = 0; i < command_length; i++) {
						command_buffer.push(from_host.read8(i));
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
