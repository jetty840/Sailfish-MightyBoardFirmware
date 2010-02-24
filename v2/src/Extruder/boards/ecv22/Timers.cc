#include "Timers.hh"

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "Configuration.hh"

volatile micros_t micros = 0L;

void startTimers() {
	// Timer 1 is for microsecond-level timing.
	// Prepare interrupt
	// CTC mode, interrupt on OCR1A, no prescaler
	TCCR1A = 0x00;
	TCCR1B = 0x09;
	TCCR1C = 0x00;
	OCR1A = INTERVAL_IN_MICROSECONDS * 16;
	TIMSK1 = 0x02; // turn on OCR1A match interrupt
	//setDebugLED(true);
}

micros_t getCurrentMicros() {
	micros_t micros_snapshot;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		micros_snapshot = micros;
	}
	return micros_snapshot;
}

ISR(TIMER1_COMPA_vect) {
	micros++;
}
