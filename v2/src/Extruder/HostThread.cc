#include "HostThread.hh"
#include "UART.hh"
#include "Timeout.hh"
#include <util/atomic.h>
#include "DebugPin.hh"
#include "DebugPacketProcessor.hh"

Timeout packet_in_timeout;

#define HOST_PACKET_TIMEOUT_MS 20
#define HOST_PACKET_TIMEOUT_MICROS (1000*HOST_PACKET_TIMEOUT_MS)

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
			setDebugLED(false);
		}
	}
	if (in.hasError()) {
		// REPORTING: report error.
		// Reset packet quickly and start handling the next packet.
		in.reset();
	}
	if (in.isFinished()) {
		out.reset();
		if (processDebugPacket(in, out)) {
			// okay, processed
		} else {
			// Unrecognized command
			out.append8(RC_CMD_UNSUPPORTED);
		}
		in.reset();
		uart[0].beginSend();
	}
}
