#include "Steppers.hh"
#include "DebugPin.hh"

namespace steppers {

class Axis {
public:
	Axis() : interface(0) {}

	Axis(StepperInterface& stepper_interface) :
		interface(&stepper_interface) {
		reset();
	}

	/// Set target coordinate and compute delta
	void setTarget(const int32_t target_in) {
		target = target_in;
		delta = target - position;
		direction = true;
		if (delta != 0) {
			interface->setEnabled(true);
		}
		if (delta < 0) {
			delta = -delta;
			direction = false;
		}
	}

	/// Define current position as the given value
	void definePosition(const int32_t position_in) {
		position = position_in;
	}

	/// Enable/disable stepper
	void enableStepper(bool enable) {
		interface->setEnabled(enable);
	}

	/// Reset to initial state
	void reset() {
		position = 0;
		minimum = 0;
		maximum = 0;
		target = 0;
		counter = 0;
		delta = 0;
	}

	void doInterrupt(int32_t intervals) {
		counter += delta;
		if (counter >= 0) {
			interface->setDirection(direction);
			interface->step();
			counter -= intervals;
			if (direction) {
				position++;
			} else {
				position--;
			}
		}
	}

	StepperInterface* interface;
	/// Current position on this axis, in steps
	volatile int32_t position;
	/// Minimum position, in steps
	int32_t minimum;
	/// Maximum position, in steps
	int32_t maximum;
	/// Target position, in steps
	volatile int32_t target;
	/// Step counter; represents the proportion of a
	/// step so far passed.  When the counter hits
	/// zero, a step is taken.
	volatile int32_t counter;
	/// Amount to increment counter per tick
	volatile int32_t delta;
	/// True for positive, false for negative
	volatile bool direction;
};

bool is_running;
int32_t intervals;
volatile int32_t intervals_remaining;
Axis axes[STEPPER_COUNT];

bool isRunning() {
	return is_running;
}

//public:
void init(Motherboard& motherboard) {
	is_running = false;
	for (int i = 0; i < STEPPER_COUNT; i++) {
		axes[i] = Axis(motherboard.getStepperInterface(i));
	}
}

void abort() {
	is_running = false;
}

/// Define current position as given point
void definePosition(const Point& position) {
	for (int i = 0; i < STEPPER_COUNT; i++) {
		axes[i].definePosition(position[i]);
	}
}

/// Get current position
const Point getPosition() {
	return Point(axes[0].position,axes[1].position,axes[2].position);
}

void setTarget(const Point& target, int32_t dda_interval) {
	int32_t max_delta = 0;
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes[i].setTarget(target[i]);
		const int32_t delta = axes[i].delta;
		// Only shut z axis on inactivity
		if (i == 2) axes[i].enableStepper(delta != 0);
		else if (delta != 0) axes[i].enableStepper(true);
		if (delta > max_delta) {
			max_delta = delta;
		}
	}
	// compute number of intervals for this move
	intervals = ((max_delta * dda_interval) / INTERVAL_IN_MICROSECONDS) + 1;
	intervals_remaining = intervals;
	const int32_t negative_half_interval = -intervals / 2;
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes[i].counter = negative_half_interval;
	}
	is_running = true;
}

/// Enable/disable the given axis.
void enableAxis(uint8_t which, bool enable) {
	axes[which].enableStepper(enable);
}

bool doInterrupt() {
	if (is_running) {
		for (int i = 0; i < STEPPER_COUNT; i++) {
			axes[i].doInterrupt(intervals);
		}
		if (--intervals_remaining == 0) {
			is_running = false;
		}
	}
	return is_running;
}

}
