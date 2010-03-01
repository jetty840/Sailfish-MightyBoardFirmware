#ifndef TOOL_THREAD_HH_
#define TOOL_THREAD_HH_

#include "Packet.hh"
#include "Timeout.hh"
#include "UART.hh"
#include <util/atomic.h>
#include "CircularBuffer.hh"

Timeout tool_packet_in_timeout;

InPacket& getToolPacket() { return uart[1].in_; }

#define TOOL_PACKET_TIMEOUT_MS 20
#define TOOL_PACKET_TIMEOUT_MICROS (1000*TOOL_PACKET_TIMEOUT_MS)

#define TOOL_QUEUE_CAPACITY 4

class ToolQueueEntry {
public:
	OutPacket* out_;
	InPacket* in_;
	ToolQueueEntry(OutPacket* out, InPacket* in) : out_(out), in_(in) {}
	ToolQueueEntry() {}
};

ToolQueueEntry tool_queue_data[TOOL_QUEUE_CAPACITY];
CircularBufferTempl<ToolQueueEntry> tool_queue(TOOL_QUEUE_CAPACITY,tool_queue_data);

bool transaction_active = false;

void queueToolTransaction(OutPacket* out, InPacket* in) {
	tool_queue.push(ToolQueueEntry(out,in));
}

Timeout tool_transaction_timeout;

void runToolSlice() {
//	setDebugLED(true);
	InPacket& slave_in = uart[1].in_;
	OutPacket& slave_out = uart[1].out_;
	if (!transaction_active) {
		if (tool_queue.isEmpty()) {
			// nothing to do, return
			return;
		} else {
			// begin next transaction
			transaction_active = true;
			tool_transaction_timeout.start(50000); // 50 ms timeout
			// copy output packet
			slave_out = *(tool_queue[0].out_);
			slave_in.reset();
			uart[1].beginSend();
		}
	} else { // transaction active
		if (slave_in.isFinished()) {
			*(tool_queue[0].in_) = slave_in;
			tool_queue.pop();
			transaction_active = false;
		} else if (tool_transaction_timeout.hasElapsed()) {
			tool_queue[0].in_->timeout();
			tool_queue.pop();
			transaction_active = false;
		}
	}
}

#endif // TOOL_THREAD_HH_
