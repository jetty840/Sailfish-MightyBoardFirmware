/*
 * PID.cc
 *
 *  Created on: Feb 22, 2010
 *      Author: phooky
 */

#include "PID.hh"

const int ERR_ACC_MAX = 0xff;
const int ERR_ACC_MIN = -0xff;

void PID::reset() {
	error_acc = 0;
	prev_input = 0;
}

int PID::calculate(const int pv) {
	int e = sp - pv;
	error_acc += e;
	// clamp the error accumulator at +-2**12
	if (error_acc > ERR_ACC_MAX) {
		error_acc = ERR_ACC_MAX;
	}
	if (error_acc < ERR_ACC_MIN) {
		error_acc = ERR_ACC_MIN;
	}
	float p_term = e * p_gain;
	float i_term = error_acc * i_gain;
	int delta = pv - prev_input;
	float d_term = delta * d_gain;

	prev_input = pv;

	return p_term + i_term + d_term;
}
