#include "Steppers.hh"
Steppers steppers;

//class Point {
//private:
//	const int32_t coordinates_[AXIS_COUNT];
//public:
//	const int32_t& operator[](unsigned int index) const { return coordinates_[index]; }
//};

Axis::Axis(Pin step_pin, Pin dir_pin, Pin enable_pin, Pin max_pin, Pin min_pin) :
		dir_pin_(dir_pin), step_pin_(step_pin), enable_pin_(enable_pin),
		max_pin_(max_pin), min_pin_(min_pin),
		position_(0), minimum_(0), maximum_(0),
		target_(0), counter_(0), delta_(0) {
	dir_pin_.setDirection(true);
	step_pin_.setDirection(true);
	enable_pin_.setDirection(true);
	max_pin_.setDirection(false);
	min_pin_.setDirection(false);
	enable_pin_.setValue(false);
}

void Axis::setTarget(const int32_t target) {
	target_ = target;
	delta_ = target - position_;
}

#if AXIS_COUNT != 3
#error Expected AXIS_COUNT of 3 for this board.
#endif

Axis axes[3] = {
		Axis(X_STEP_PIN,X_DIR_PIN,X_ENABLE_PIN,X_MIN_PIN,X_MAX_PIN),
		Axis(Y_STEP_PIN,Y_DIR_PIN,Y_ENABLE_PIN,Y_MIN_PIN,Y_MAX_PIN),
		Axis(Z_STEP_PIN,Z_DIR_PIN,Z_ENABLE_PIN,Z_MIN_PIN,Z_MAX_PIN)
};

//public:
Steppers::Steppers() : is_running_(false), is_paused_(false), axes_(axes) {}

void Steppers::abort() {
	is_running_ = false;
	is_paused_ = false;
}

void Steppers::setTarget(const Point& target, int32_t dda_interval) {
	int32_t max_delta = 0;
	for (int i =0; i < AXIS_COUNT; i++) {
		axes_[i].setTarget(target[i]);
		const int32_t delta = axes[i].delta_;
		const int32_t abs_delta = delta<0?-delta:delta;
		if (abs_delta > max_delta) { max_delta = abs_delta; }
	}
	// compute number of intervals for this move
	intervals_ = max_delta * dda_interval / INTERVAL_IN_MICROSECONDS;
	//intervals_ = 0x0fffffL;
	intervals_remaining_ = intervals_;
	for (int i =0; i < AXIS_COUNT; i++) {
		axes_[i].counter_ = -intervals_ /2;
	}
	// Prepare interrupt
	// CTC mode, interrupt on OCR1A, clk/256 prescaler
	TCCR1A = 0x00;
	TCCR1B = 0x0c;
	TCCR1C = 0x00;
	OCR1A = INTERVAL_IN_MICROSECONDS * 16 / 256;
	TIMSK1 = 0x02; // turn on OCR1A match interrupt
	is_running_ = true;
}

void Steppers::doInterrupt() {
	if (is_paused_ || !is_running_) { return; }
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes_[i].counter_ += axes_[i].delta_;
		if (axes_[i].counter_ >= 0) {
			axes_[i].dir_pin_.setValue(true);
			axes_[i].step_pin_.setValue(true);
			axes_[i].counter_ -= intervals_;
			axes_[i].position_++;
			axes_[i].step_pin_.setValue(false);
			axes_[i].dir_pin_.setValue(false);
		}
	}
	if (--intervals_remaining_ == 0) {
		is_running_ = false;
	}
}

ISR(TIMER1_COMPA_vect) {
	steppers.doInterrupt();
}
