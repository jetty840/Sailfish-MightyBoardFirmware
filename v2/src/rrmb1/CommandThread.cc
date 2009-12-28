#include "CommandThread.hh"
#include "CommandQueue.hh"
#include "Steppers.hh"
#include "Commands.hh"

namespace CommandThread {
bool running = false;
bool paused = false;
}
;

void pauseCommandThread(bool paused) {
	CommandThread::paused = paused;
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
}

void stopCommandThread() {
	TIMSK1 = 0x00; // turn off OCR1A match interrupt
	CommandThread::running = false;
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

ISR(TIMER1_COMPA_vect) {
	if (CommandThread::paused) return;
	if (!steppers.doInterrupt()) {
		// process next command on the queue
		if (command_buffer.getLength() > 0) {
			uint8_t command = command_buffer[0];
			if (command == HOST_CMD_QUEUE_POINT_ABS) {
				// check for completion
				if (command_buffer.getLength() < 17) {
					// incomplete command; wait for the rest.
					return;
				}
				command_buffer.pop(); // remove the command code
				int32_t x = pop32();
				int32_t y = pop32();
				int32_t z = pop32();
				int32_t dda = pop32();
				steppers.setTarget(Point(x,y,z),dda);
			} else if (command == HOST_CMD_SET_POSITION) {
				// check for completion
				if (command_buffer.getLength() < 13) {
					// incomplete command; wait for the rest.
					return;
				}
				command_buffer.pop(); // remove the command code
				int32_t x = pop32();
				int32_t y = pop32();
				int32_t z = pop32();
				steppers.definePosition(Point(x,y,z));
				//steppers.setTarget(Point(500,500,0),500);
			}

		}
	}
}
