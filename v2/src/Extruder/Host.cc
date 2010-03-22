#include "Host.hh"
#include "UART.hh"
#include "Timeout.hh"
#include <util/atomic.h>
#include "DebugPin.hh"
#include "QueryPacketProcessor.hh"
#include "DebugPacketProcessor.hh"
#include "Configuration.hh"
#include "ExtruderBoard.hh"

Timeout packet_in_timeout;

#define HOST_PACKET_TIMEOUT_MS 20
#define HOST_PACKET_TIMEOUT_MICROS (1000*HOST_PACKET_TIMEOUT_MS)

void runHostSlice() {
	UART& uart = ExtruderBoard::getBoard().getHostUART();
	InPacket& in = uart.in_;
	OutPacket& out = uart.out_;
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
		in.reset();
	}
	if (in.isFinished()) {
		packet_in_timeout.abort();
		out.reset();
		// Check if this is our packet.  Strip out the ID.
		if (in.read8(0) == DEVICE_ID) {
			if (processDebugPacket(in, out)) {
				// okay, processed
			} else if (processQueryPacket(in, out)) {
				// okay, processed
			} else {
				// Unrecognized command
				out.append8(RC_CMD_UNSUPPORTED);
			}
		}
		in.reset();
		uart.beginSend();
	}
}
