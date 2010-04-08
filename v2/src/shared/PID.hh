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
 */

#ifndef PID_HH_
#define PID_HH_

/// This simplified PID controller makes several assumptions:
/// * The output range is limited to 0-255.
class PID {
private:
    float p_gain; // proportional gain
    float i_gain; // integral gain
    float d_gain; // derivative gain

    int prev_error; // previous input for calculating delta
    int error_acc;  // accumulated error, for calculating integral

    int sp; // set point
public:
    void setPGain(const float p_gain_in) { p_gain = p_gain_in; }
    void setIGain(const float i_gain_in) { i_gain = i_gain_in; }
    void setDGain(const float d_gain_in) { d_gain = d_gain_in; }

    void setTarget(const int target) { sp = target; }
    const int getTarget() const { return sp; }

    void reset();
    /// PV is the process value; that is, the measured value
    /// Returns the new value of the manipulated value; that is, the output
    int calculate(int pv);
};

#endif /* PID_HH_ */
