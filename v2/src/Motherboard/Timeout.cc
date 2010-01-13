#include "Timeout.hh"
#include "Timers.hh"

Timeout::Timeout() : active_(false) {}

Timeout::Timeout(int32_t duration_micros) {
	active_ = true;
	end_stamp_micros_ = getCurrentMicros() + duration_micros;
}

bool Timeout::hasElapsed() {
	if (active_) {
		if ((end_stamp_micros_ - getCurrentMicros()) <= 0) {
			active_ = false;
		}
	}
	return active_;
}
