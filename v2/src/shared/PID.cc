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

#include "PID.hh"

const int ERR_ACC_MAX = 0xff;
const int ERR_ACC_MIN = -0xff;

void PID::reset() {
	error_acc = 0;
	prev_error = 0;
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
	int delta = e - prev_error;
	float d_term = delta * d_gain;

	prev_error = e;

	return p_term + i_term + d_term;
}
