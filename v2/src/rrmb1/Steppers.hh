#ifndef STEPPERS_HH_
#define STEPPERS_HH_

#include "Platform.hh"
#include <stdint.h>
#include "AvrPort.hh"

#ifndef AXIS_COUNT
#define AXIS_COUNT 3
#endif

/// We're moving to a fixed-length interval.  This gives us approximately 8000
/// instructions per interval, and ~2000 steps/second.
#define INTERVAL_IN_MICROSECONDS 32

class Point {
private:
	int32_t coordinates_[AXIS_COUNT];
public:
	Point() {}
	Point(int32_t x, int32_t y, int32_t z) {
		coordinates_[0] = x;
		coordinates_[1] = y;
		coordinates_[2] = z;
	}
	const int32_t& operator[](unsigned int index) const { return coordinates_[index]; }
};

class Steppers;

class Axis {
	friend class Steppers;
private:
	Pin dir_pin_;
	Pin step_pin_;
	Pin enable_pin_;
	Pin max_pin_;
	Pin min_pin_;
public:
	Axis(Pin step_pin, Pin dir_pin, Pin enable_pin, Pin max_pin, Pin min_pin);
	/// Current position on this axis, in steps
	volatile int32_t position_;
	/// Minimum position, in steps
	int32_t minimum_;
	/// Maximum position, in steps
	int32_t maximum_;
	/// Target position, in steps
	volatile int32_t target_;
	/// Step counter; represents the proportion of a
	/// step so far passed.  When the counter hits
	/// zero, a step is taken.
	volatile int32_t counter_;
	/// Amount to increment counter per tick
	volatile int32_t delta_;
	/// True for positive, false for negative
	volatile bool direction_;
	/// Set target coordinate and compute delta
	void setTarget(const int32_t target);
};

class Steppers {
private:
	bool is_running_;
	bool is_paused_;

	Axis* axes_;
	int32_t intervals_;
	volatile int32_t intervals_remaining_;
public:
	/// Create the Stepper subsystem.
	Steppers();
	/// Returns true if the stepper subsystem is running.  If the
	/// stepper subsystem is idle, returns false.  Will return true
	/// if the system is running but paused.
	bool isRunning() const { return is_running_; }
	/// Abort the current motion and set the stepper subsystem to
	/// the not-running state.
	void abort();
	/// Returns true if the stepper subsystem has been paused.
	bool isPaused() const { return is_paused_; }
	/// Set the pause mode of the stepper subsystem.
	void setPaused(bool paused) { is_paused_ = paused; }
	/// Set current target
	void setTarget(const Point& target, int32_t dda_interval);
	/// Handle interrupt
	void doInterrupt();
};

extern Steppers steppers;

#endif /* STEPPERS_HH_ */
