/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#define __STDC_LIMIT_MACROS
#include "Steppers.hh"
#include "StepperAxis.hh"
#include <stdint.h>

namespace steppers {


volatile bool is_running;
int32_t intervals;
volatile int32_t intervals_remaining;
StepperAxis axes[STEPPER_COUNT];
volatile bool is_homing;

bool holdZ = false;

bool isRunning() {
	return is_running || is_homing;
}

//public:
void init(Motherboard& motherboard) {
	is_running = false;
	for (int i = 0; i < STEPPER_COUNT; i++) {
                axes[i] = StepperAxis(motherboard.getStepperInterface(i));
	}
}

void abort() {
	is_running = false;
	is_homing = false;
}

/// Define current position as given point
void definePosition(const Point& position) {
	for (int i = 0; i < STEPPER_COUNT; i++) {
		axes[i].definePosition(position[i]);
	}
}

/// Get current position
const Point getPosition() {
#if STEPPER_COUNT > 3
	return Point(axes[0].position,axes[1].position,axes[2].position,axes[3].position,axes[4].position);
#else
	return Point(axes[0].position,axes[1].position,axes[2].position);
#endif
}

void setHoldZ(bool holdZ_in) {
	holdZ = holdZ_in;
}

void setTarget(const Point& target, int32_t dda_interval) {
	int32_t max_delta = 0;
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes[i].setTarget(target[i], false);
		const int32_t delta = axes[i].delta;
		// Only shut z axis on inactivity
		if (i == 2 && !holdZ) axes[i].enableStepper(delta != 0);
		else if (delta != 0) axes[i].enableStepper(true);
		if (delta > max_delta) {
			max_delta = delta;
		}
	}
	// compute number of intervals for this move
	intervals = ((max_delta * dda_interval) / INTERVAL_IN_MICROSECONDS);
	intervals_remaining = intervals;
	const int32_t negative_half_interval = -intervals / 2;
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes[i].counter = negative_half_interval;
	}
	is_running = true;
}

void setTargetNew(const Point& target, int32_t us, uint8_t relative) {
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes[i].setTarget(target[i], (relative & (1 << i)) != 0);
		// Only shut z axis on inactivity
		const int32_t delta = axes[i].delta;
		if (i == 2 && !holdZ) {
			axes[i].enableStepper(delta != 0);
		} else if (delta != 0) {
			axes[i].enableStepper(true);
		}
	}
	// compute number of intervals for this move
	intervals = us / INTERVAL_IN_MICROSECONDS;
	intervals_remaining = intervals;
	const int32_t negative_half_interval = -intervals / 2;
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes[i].counter = negative_half_interval;
	}
	is_running = true;
}

/// Start homing
void startHoming(const bool maximums, const uint8_t axes_enabled, const uint32_t us_per_step) {
	intervals_remaining = INT32_MAX;
	intervals = us_per_step / INTERVAL_IN_MICROSECONDS;
	const int32_t negative_half_interval = -intervals / 2;
	for (int i = 0; i < AXIS_COUNT; i++) {
		axes[i].counter = negative_half_interval;
		if ((axes_enabled & (1<<i)) != 0) {
			axes[i].setHoming(maximums);
		} else {
			axes[i].delta = 0;
		}
	}
	is_homing = true;
}

/// Enable/disable the given axis.
void enableAxis(uint8_t index, bool enable) {
        if (index < STEPPER_COUNT) {
                axes[index].enableStepper(enable);
	}
}

bool doInterrupt() {
	if (is_running) {
		if (intervals_remaining-- == 0) {
			is_running = false;
		} else {
			for (int i = 0; i < STEPPER_COUNT; i++) {
				axes[i].doInterrupt(intervals);
			}
		}
		return is_running;
	} else if (is_homing) {
		is_homing = false;
		for (int i = 0; i < STEPPER_COUNT; i++) {
			bool still_homing = axes[i].doHoming(intervals);
			is_homing = still_homing || is_homing;
		}
		return is_homing;
	}
	return false;
}

}
