#include "Steppers.hh"
#include "DebugPin.hh"

Steppers steppers;

//class Point {
//private:
//	const int32_t coordinates_[AXIS_COUNT];
//public:
//	const int32_t& operator[](unsigned int index) const { return coordinates_[index]; }
//};

// yay
#define ENABLE_ON false
#define ENABLE_OFF true

Axis::Axis(Pin step_pin, Pin dir_pin, Pin enable_pin, Pin max_pin, Pin min_pin) :
	dir_pin_(dir_pin), step_pin_(step_pin), enable_pin_(enable_pin), max_pin_(
			max_pin), min_pin_(min_pin), position_(0), minimum_(0),
			maximum_(0), target_(0), counter_(0), delta_(0) {
	dir_pin_.setDirection(true);
	step_pin_.setDirection(true);
	enable_pin_.setDirection(true);
	max_pin_.setDirection(false);
	min_pin_.setDirection(false);
	enable_pin_.setValue(ENABLE_OFF);
}

/// Define current position as the given value
void Axis::definePosition(const int32_t position) {
	position_ = position;
}

void Axis::setTarget(const int32_t target) {
	target_ = target;
	delta_ = target - position_;
	direction_ = true;
	if (delta_ != 0) {
		enable_pin_.setValue(ENABLE_ON);
	}
	if (delta_ < 0) {
		delta_ = -delta_;
		direction_ = false;
	}
}

void Axis::enableStepper(bool enable) {
	enable_pin_.setValue(enable?ENABLE_ON:ENABLE_OFF);
}

#if AXIS_COUNT != 3
#error Expected AXIS_COUNT of 3 for this board.
#endif

Axis axes[3] = {
		Axis(X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN, X_MIN_PIN, X_MAX_PIN), Axis(
				Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN, Y_MIN_PIN, Y_MAX_PIN),
		Axis(Z_STEP_PIN, Z_DIR_PIN, Z_ENABLE_PIN, Z_MIN_PIN, Z_MAX_PIN) };

//public:
Steppers::Steppers() :
	is_running_(false), axes_(axes) {
}

void Steppers::abort() {
	is_running_ = false;
}

/// Define current position as given point
void Steppers::definePosition(const Point& position) {
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes_[i].definePosition(position[i]);
	}
}

/// Get current position
const Point Steppers::getPosition() {
	return Point(axes_[0].position_,axes_[1].position_,axes_[2].position_);
}

void Steppers::setTarget(const Point& target, int32_t dda_interval) {
	int32_t max_delta = 0;
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes_[i].setTarget(target[i]);
		const int32_t delta = axes[i].delta_;
		axes_[i].enable_pin_.setValue(delta == 0);
		if (delta > max_delta) {
			max_delta = delta;
		}
	}
	// compute number of intervals for this move
	intervals_ = ((max_delta * dda_interval) / INTERVAL_IN_MICROSECONDS) + 1;
	intervals_remaining_ = intervals_;
	const int32_t negative_half_interval = -intervals_ / 2;
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes_[i].counter_ = negative_half_interval;
	}
	// Prepare interrupt
	// CTC mode, interrupt on OCR1A, no prescaler
	is_running_ = true;
}

/// Enable/disable the given axis.
void Steppers::enableAxis(uint8_t which, bool enable) {
	axes_[which].enableStepper(enable);
}

bool Steppers::doInterrupt() {
	if (is_running_) {
		for (int i = 0; i < AXIS_COUNT; i++) {
			axes_[i].counter_ += axes_[i].delta_;
			if (axes_[i].counter_ >= 0) {
				axes_[i].dir_pin_.setValue(axes_[i].direction_);
				axes_[i].step_pin_.setValue(true);
				axes_[i].counter_ -= intervals_;
				if (axes_[i].direction_) {
					axes_[i].position_++;
				} else {
					axes_[i].position_--;
				}
				axes_[i].step_pin_.setValue(false);
			}
		}
		if (--intervals_remaining_ == 0) {
			is_running_ = false;
		}
	}
	return is_running_;
}
