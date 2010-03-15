#include "Timeout.hh"
#include "Motherboard.hh"

Timeout::Timeout() : active_(false), elapsed_(false) {}

void Timeout::start(uint32_t duration_micros) {
	active_ = true;
	elapsed_ = false;
	end_stamp_micros_ = Motherboard::getBoard().getCurrentMicros() + duration_micros;
}

bool Timeout::hasElapsed() {
	if (active_ && !elapsed_) {
		if ((end_stamp_micros_ - Motherboard::getBoard().getCurrentMicros()) <= 0) {
			active_ = false;
			elapsed_ = true;
		}
	}
	return elapsed_;
}

void Timeout::abort() {
	active_ = false;
}
