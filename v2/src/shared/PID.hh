/*
 * PID.hh
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
    float p_gain_; // proportional gain
    float i_gain_; // integral gain
    float d_gain_; // derivative gain

    int prev_input_; // previous input for calculating delta
    int error_acc_;  // accumulated error, for calculating integral

    int sp_; // set point
public:
    void setPGain(const float p_gain) { p_gain_ = p_gain; }
    void setIGain(const float i_gain) { i_gain_ = i_gain; }
    void setDGain(const float d_gain) { d_gain_ = d_gain; }

    void setTarget(const int target) { sp_ = target; }
    const int getTarget() const { return sp_; }

    void reset();
    /// PV is the process value; that is, the measured value
    /// Returns the new value of the manipulated value; that is, the output
    int calculate(int pv);
};

#endif /* PID_HH_ */
