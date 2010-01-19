#ifndef TOOL_THREAD_HH_
#define TOOL_THREAD_HH_

#include "Packet.hh"
#include "Timeout.hh"
#include "UART.hh"
#include <util/atomic.h>
#include "CircularBuffer.hh"

Timeout tool_packet_in_timeout;

#define TOOL_PACKET_TIMEOUT_MS 20
#define TOOL_PACKET_TIMEOUT_MICROS (1000*TOOL_PACKET_TIMEOUT_MS)

class ToolQueueEntry {
public:
	OutPacket* out_;
	InPacket* in_;
	ToolQueueEntry() : out_(0), in_(0) {}
	ToolQueueEntry(OutPacket* out, InPacket* in) : out_(out), in_(in) {}
};

#define TOOL_QUEUE_CAPACITY 4

ToolQueueEntry tool_queue_data[TOOL_QUEUE_CAPACITY];
CircularBufferTempl<ToolQueueEntry> tool_queue(TOOL_QUEUE_CAPACITY,tool_queue_data);

bool transaction_active = false;

void queueToolTransaction(OutPacket* out, InPacket* in) {
	tool_queue.push(ToolQueueEntry(out,in));
}

void runToolSlice() {
	InPacket& in = uart[1].in_;
	OutPacket& out = uart[1].out_;
	if (!transaction_active) {
		if (tool_queue.isEmpty()) {
			// nothing to do, return
			return;
		} else {
			// begin next transaction
			transaction_active = true;
			// copy output packet
			out = *(tool_queue[0].out_);
			if (out.getLength() != tool_queue[0].out_->getLength()) setDebugLED(false);
			in.reset();
			uart[1].beginSend();
		}
	} else { // transaction active
		if (out.isSending()) {
			// still sending; wait until send is complete before considering new host packets.
			return;
		} else {
			// waiting for reply.
			// TODO: add toolhead timeout
			// if (in.isStarted()) { setDebugLED(false); }
			if (in.getErrorCode() == PacketError::NOISE_BYTE) in.reset();
			//	in.reset();
			if (in.isFinished() || in.hasError()) {
				// finish transaction
				transaction_active = false;
//				if (in.isFinished()) setDebugLED(false);
				*(tool_queue[0].in_) = in;
//				if (tool_queue[0].in_->isFinished()) setDebugLED(false);
				tool_queue.pop();
				in.reset();
			}
		}
	}
}

#endif // TOOL_THREAD_HH_
