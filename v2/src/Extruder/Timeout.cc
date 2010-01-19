#include "Timeout.hh"
#include "Timers.hh"

Timeout::Timeout() : active_(false), elapsed_(false) {}

Timeout::Timeout(int32_t duration_micros) {
	active_ = true;
	elapsed_ = false;
	end_stamp_micros_ = getCurrentMicros() + duration_micros;
}

bool Timeout::hasElapsed() {
	if (active_ && !elapsed_) {
		if ((end_stamp_micros_ - getCurrentMicros()) <= 0) {
			active_ = false;
			elapsed_ = true;
		}
	}
	return elapsed_;
}
