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
	error_acc_ = 0;
	prev_input_ = 0;
}

int PID::calculate(const int pv) {
	int e = sp_ - pv;
	error_acc_ += e;
	// clamp the error accumulator at +-2**12
	if (error_acc_ > ERR_ACC_MAX) {
		error_acc_ = ERR_ACC_MAX;
	}
	if (error_acc_ < ERR_ACC_MIN) {
		error_acc_ = ERR_ACC_MIN;
	}
	float p_term = e * p_gain_;
	float i_term = error_acc_ * i_gain_;
	int delta = pv - prev_input_;
	float d_term = delta * d_gain_;

	prev_input_ = pv;

	return p_term + i_term + d_term;
}
