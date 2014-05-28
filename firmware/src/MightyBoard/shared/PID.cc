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
 *
 * This simplified PID controller was written with reference to:
 * * The Heater.h implementation (lookup credits)
 * * Brett Beauregard's Arduino PID implementation
 *  Created on: Feb 19, 2010
 *      Author: phooky
 *
 */

#include <stdlib.h>
#include "PID.hh"

#define ERR_ACC_MAX 256
#define ERR_ACC_MIN -ERR_ACC_MAX

// scale the output term to account for our fixed-point bounds
#define OUTPUT_SCALE 2

PID::PID() {
    reset();
}

void PID::reset() {
	sp = 0;
	p_gain = i_gain = d_gain = 0;

	reset_state();
}

void PID::reset_state() {
	error_acc = 0;
	prev_error = 0;
	delta_idx = 0;

// Assume that DELTA_SAMPLES is 4 -- saves 24 bytes to eliminate this loop
//	for (delta_idx = 0; delta_idx < DELTA_SAMPLES; delta_idx++)
//		delta_history[delta_idx] = 0;
	delta_history[0] = 0;
	delta_history[1] = 0;
	delta_history[2] = 0;
	delta_history[3] = 0;

	delta_idx = 0;
	delta_summation = 0;

#if defined(SUPPORT_GET_PID_STATE)
	last_output = 0;
#endif
}

// We're modifying the way we compute delta by averaging the deltas over a
// series of samples.  This helps us get a reasonable delta despite the discrete
// nature of the samples; on average we will get a delta of maybe 1/deg/second,
// which will give us a delta impulse for that one calculation round and then
// the D term will immediately disappear.  By averaging the last N deltas, we
// allow changes to be registered rather than get subsumed in the sampling noise.
int PID::calculate(const float pv) {
	float e = sp - pv;
	error_acc += e;
	// Clamp the error accumulator at accepted values.
	// This will help control overcorrection for accumulated error during the run-up
	// and allow the I term to be integrated away more quickly as we approach the
	// setpoint.
	if (error_acc > ERR_ACC_MAX)
		error_acc = ERR_ACC_MAX;
	else if (error_acc < ERR_ACC_MIN)
		error_acc = ERR_ACC_MIN;
	float p_term = (float)e * p_gain;
	float i_term = (float)error_acc * i_gain;
	float delta = e - prev_error;
	// Add to delta history
	delta_summation -= delta_history[delta_idx];
	delta_history[delta_idx] = delta;
	delta_summation += (float)delta;
	delta_idx = (delta_idx+1) % DELTA_SAMPLES;
	// Use the delta over the whole window
	float d_term = delta_summation * d_gain;

	prev_error = e;

#if !defined(SUPPORT_GET_PID_STATE)
	int last_output;
#endif
	last_output = ((int)(p_term + i_term + d_term))*OUTPUT_SCALE;

	return last_output;
}

void PID::setTarget(const int target) {
    if (abs(sp - target) > 10)
	reset_state();
    sp = target;
}
