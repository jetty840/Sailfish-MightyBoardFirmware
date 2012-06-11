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

#include <stdint.h>

/// Number of delta samples to
#define DELTA_SAMPLES 4

/// The PID controller module implements a simple PID controller.
/// \ingroup SoftwareLibraries
class PID {
private:
    float p_gain; ///< proportional gain
    float i_gain; ///< integral gain
    float d_gain; ///< derivative gain

    /// Data for approximating d (smoothing to handle discrete nature of sampling).
    /// See PID.cc for a description of why we do this.
    int16_t delta_history[DELTA_SAMPLES];
    float delta_summation;      ///< ?
    uint8_t delta_idx;          ///< Current index in the delta history buffer
    int prev_error;             ///< Previous input for calculating next delta
    int error_acc;              ///< Accumulated error, for calculating integral

    int sp;                     ///< Process set point
    int last_output;            ///< Last output of the PID controller

public:
    /// Initialize the PID module
    PID();

    /// Set the P term of the PID controller
    /// \param[in] p_gain_in New proportional gain term
    void setPGain(const float p_gain_in) { p_gain = p_gain_in; }

    /// Set the I term of the PID controller
    /// \param[in] i_gain_in New integration gain term
    void setIGain(const float i_gain_in) { i_gain = i_gain_in; }

    /// Set the D term of the PID controller
    /// \param[in] d_gain_in New derivative gain term
    void setDGain(const float d_gain_in) { d_gain = d_gain_in; }

    /// Set the setpoint of the PID controller
    /// \param[in] target New PID controller target
    void setTarget(const int target);

    /// Get the current PID target
    /// \return Current setpoint
    const int getTarget() const { return sp; }

    /// Reset the PID to board-on values
    void reset();

    /// Reset only the PID control loop variables
    void reset_state();

    /// Calculate the next cycle of the PID loop.
    /// \param[in] pv Process value (measured value from the sensor)
    /// \return output value (used to control the output)
    int calculate(int pv);

    /// Get the current value of the error term
    /// \return Error term
    int getErrorTerm();

    /// Get the current value of the delta term
    /// \return Delta term
    int getDeltaTerm();

    /// Get the last process output value
    /// \return Last process output value
    int getLastOutput();
};

#endif /* PID_HH_ */
