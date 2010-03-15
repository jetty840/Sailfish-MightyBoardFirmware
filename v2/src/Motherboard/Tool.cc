#include "Tool.hh"
#include "Timeout.hh"
#include "UART.hh"

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
		} else if (timeout.hasElapsed()) {
			uart[1].in_.timeout();
			transaction_active = false;
		}
	}
}

}
