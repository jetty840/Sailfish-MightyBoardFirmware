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

#ifndef STEPPERS_HH_
#define STEPPERS_HH_

#include "Configuration.hh"
#include <stdint.h>
#include "Pin.hh"
#include "Command.hh"
#include "Point.hh"

#include "Types.hh"
#include "Motherboard.hh"

namespace steppers {

    /// Initialize the stepper subsystem.
    /// \param[in] motherboard Motherboard to attach the steppers to.
    void init(Motherboard& motherboard);

    /// Check if the stepper subsystem is running
    /// \return True if the stepper subsystem is running or paused. False
    ///         otherwise.
    bool isRunning();

    /// Abort the current motion and set the stepper subsystem to
    /// the not-running state.
    void abort();

    /// Enable/disable the given axis.
    /// \param[in] index Index of the axis to enable or disable
    /// \param[in] enable If true, enable the axis. If false, disable.
    void enableAxis(uint8_t index, bool enable);

    /// Instruct the stepper subsystem to move the machine to the
    /// given position.
    /// \param[in] target Position to move to
    /// \param[in] dda_interval Motion speed, in us per step.
    void setTarget(const Point& target, int32_t dda_interval);

    /// Instruct the stepper subsystem to move the machine to the
    /// given position.
    /// \param[in] target Position to move to
    /// \param[in] ms Duration of the move, in milliseconds
    /// \param[in] relative Bitfield specifying whether each axis should
    ///                     interpret the new position as absolute or
    ///                     relative.
    void setTargetNew(const Point& target,
                      int32_t ms,
                      uint8_t relative =0);

    /// Home one or more axes
    /// \param[in] maximums If true, home in the positive direction
    /// \param[in] axes_enabled Bitfield specifiying which axes to
    ///                         home
    /// \param[in] us_per_step Homing speed, in us per step
    void startHoming(const bool maximums,
                     const uint8_t axes_enabled,
                     const uint32_t us_per_step);

    /// Reset the current system position to the given point
    /// \param[in] position New system position
    void definePosition(const Point& position);

    /// Handle interrupt.
    /// \return True if the stepper subsystem is currently in motion.
    bool doInterrupt();

    /// Get the current system position
    /// \return The current machine position.
    const Point getPosition();

    /// Control whether the Z axis should stay enabled during the entire
    /// build (defaults to off). This is useful for machines that have
    /// a z-axis that might slip if the motor does not stay enagaged.
    /// \param[in] holdZ If true, the Z axis stepper will stay enabled
    ///                  through the entire build. If false, it will be
    ///                  disabled when not moving.
    void setHoldZ(bool holdZ);
};

#endif // STEPPERS_HH_
