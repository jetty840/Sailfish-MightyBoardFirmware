#include "CommandThread.hh"
#include "CommandQueue.hh"
#include "Steppers.hh"
#include "Commands.hh"
#include "DebugPin.hh"

namespace CommandThread {
bool running = false;
bool paused = false;
}
;

bool isCommandThreadPaused() {
	return CommandThread::paused;
}

void pauseCommandThread(bool paused) {
	CommandThread::paused = paused;
	//setDebugLED(!paused);
}

void startCommandThread() {
	// Prepare interrupt
	// CTC mode, interrupt on OCR1A, no prescaler
	TCCR1A = 0x00;
	TCCR1B = 0x09;
	TCCR1C = 0x00;
	OCR1A = INTERVAL_IN_MICROSECONDS * 16;
	TIMSK1 = 0x02; // turn on OCR1A match interrupt
	CommandThread::running = true;
	//setDebugLED(true);
}

void stopCommandThread() {
	TIMSK1 = 0x00; // turn off OCR1A match interrupt
	CommandThread::running = false;
	//setDebugLED(false);
}

int32_t pop32() {
	union {
		// AVR is little-endian
		int32_t a;
		struct {
			uint8_t data[4];
		} b;
	} shared;
	shared.b.data[0] = command_buffer.pop();
	shared.b.data[1] = command_buffer.pop();
	shared.b.data[2] = command_buffer.pop();
	shared.b.data[3] = command_buffer.pop();
	return shared.a;
}

uint32_t delay_counter = 0;
bool in_delay = false;

// This ISR is marked as non-blocking.  This will prevent it from accidentally
// stomping on any incoming serial data.
ISR(TIMER1_COMPA_vect, ISR_NOBLOCK) {
	if (CommandThread::paused) return;
	// This can potentially be a lengthy operation, so we disable
	// our own interrupt until we're done.
	TIMSK1 = 0x00; // turn off OCR1A match interrupt
	if (in_delay) {
		delay_counter--;
		if (delay_counter == 0) {
			in_delay = false;
		}
//	} else if (tools.doInterrupt()) {
		// If the tool subsystem indicates that we should block,
		// then, well, block.
	} else if (!steppers.doInterrupt()) {
		// process next command on the queue.
		if (command_buffer.getLength() > 0) {
			uint8_t command = command_buffer[0];
			if (command == HOST_CMD_QUEUE_POINT_ABS) {
				// check for completion
				if (command_buffer.getLength() >= 17) {
					command_buffer.pop(); // remove the command code
					int32_t x = pop32();
					int32_t y = pop32();
					int32_t z = pop32();
					int32_t dda = pop32();
					steppers.setTarget(Point(x,y,z),dda);
				}
			} else if (command == HOST_CMD_SET_POSITION) {
				// check for completion
				if (command_buffer.getLength() >= 13) {
					command_buffer.pop(); // remove the command code
					int32_t x = pop32();
					int32_t y = pop32();
					int32_t z = pop32();
					steppers.definePosition(Point(x,y,z));
				}
			} else if (command == HOST_CMD_DELAY) {
				if (command_buffer.getLength() >= 5) {
					in_delay = true;
					command_buffer.pop(); // remove the command code
					uint32_t microseconds = pop32();
					delay_counter = microseconds / INTERVAL_IN_MICROSECONDS;
					delay_counter = 1000;
					if (delay_counter == 0) in_delay = false;
				}
			} else if (command == HOST_CMD_DELAY) {
			}
		}
	}
	// re-enable the command thread interrupt
	TIMSK1 = 0x02; // turn on OCR1A match interrupt
}
