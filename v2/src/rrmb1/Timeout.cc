#include "Timeout.hh"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#define MAX_TIMEOUTS 4

class TimeoutEntry {
private:
	Timeout* volatile timeout_ptr_;
	volatile int16_t ms_remaining_;
public:
	TimeoutEntry() : timeout_ptr_(0), ms_remaining_(0) {}
	TimeoutEntry(Timeout* timeout, uint16_t ms) : timeout_ptr_(timeout), ms_remaining_(ms) {}
	// Return true if timeout was triggered
	bool processTime(int16_t ms) {
		ms_remaining_ -= ms;
		return (ms_remaining_ <= 0);
	}
	bool isNull() const {
		return timeout_ptr_ == 0;
	}
	bool matches(Timeout& timeout) const {
		return &timeout == timeout_ptr_;
	}
	Timeout* getTimeoutPtr() const {
		return timeout_ptr_;
	}
};

TimeoutEntry timeout_table[MAX_TIMEOUTS];

void TimeoutManager::addTimeout(Timeout& timeout, uint16_t ms) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		for (int i = 0; i < MAX_TIMEOUTS; i++) {
			TimeoutEntry& entry = timeout_table[i];
			if (entry.matches(timeout) ||  entry.isNull()) {
				entry = TimeoutEntry(&timeout,ms);
				return;
			}
		}
	}
}

void TimeoutManager::cancelTimeout(Timeout& timeout) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		int index_target = -1;
		int index_last = -1;
		for (int i = 0; i < MAX_TIMEOUTS; i++) {
			TimeoutEntry& entry = timeout_table[i];
			if (!entry.isNull()) { index_last = i; }
			if (entry.matches(timeout)) { index_target = i; }
		}
		if (index_target != -1) {
			if (index_target != index_last) {
				timeout_table[index_target] = timeout_table[index_last];
			}
			timeout_table[index_last] = TimeoutEntry();
		}
	}
}

void TimeoutManager::init() {
	// Set up timer0 to generate an interrupt every other ms.
	// Mode: CTC
	// Prescaler: 256
	// Top: 125
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS02);
	OCR0A = 125;
	TIMSK0 = _BV(OCIE0A);
}

#define MS_PER_INTERRUPT 2

ISR(TIMER0_COMPA_vect) {
	for (int i = 0; i < MAX_TIMEOUTS; i++) {
		TimeoutEntry& entry = timeout_table[i];
		if (entry.isNull()) { break; }
		bool timed_out = entry.processTime(MS_PER_INTERRUPT);
		if (timed_out) {
			Timeout* timeout_ptr = entry.getTimeoutPtr();
			TimeoutManager::cancelTimeout(*timeout_ptr);
			i--; // slot has been emptied
			timeout_ptr->timeout();
		}
	}
}
