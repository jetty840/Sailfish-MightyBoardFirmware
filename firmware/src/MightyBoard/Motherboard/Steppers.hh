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

#ifndef SIMULATOR
#include "Configuration.hh"
#include "Types.hh"
#include <stdlib.h>
#include "Pin.hh"
#include "Command.hh"
#include "Point.hh"
#include "StepperAccel.hh"
#include "Motherboard.hh"
#else
#include "Configuration.hh"
#include "Types.hh"
#include <stdlib.h>
#include "Command.hh"
#include "Point.hh"
#include "StepperAccel.hh"
#endif

#ifdef DEBUG_ONSCREEN
extern volatile float debug_onscreen1, debug_onscreen2;
#endif

namespace steppers {

    extern bool acceleration;
    extern bool extruder_hold[EXTRUDERS];
    extern uint8_t alterSpeed;
    extern uint8_t toolIndex;
    extern FPTYPE axis_steps_per_unit_inverse[STEPPER_COUNT];
    extern FPTYPE speedFactor;

    /// Check if the stepper subsystem is running
    /// \return True if the stepper subsystem is running or paused. False
    ///         otherwise.
    bool isRunning();

    /// re-initialize stepper pins - disable all axes
    void reset();

    /// Abort the current motion and set the stepper subsystem to
    /// Initialize the stepper subsystem.
    /// \param[in] motherboard Motherboard to attach the steppers to.
    void init();

    /// the not-running state.
    void abort();

    /// Reset the current system position to the given point
    /// \param[in] position New system position
	void definePosition(const Point& position, bool home);

    /// Get the current system position
    /// \return The current machine position.
    const Point getPlannerPosition();

    /// Get current position
    /// When accelerated, this is the position right now
    const Point getStepperPosition(uint8_t *toolIndex);

    /// Instruct the stepper subsystem to move the machine to the
    /// given position.
    /// \param[in] target Position to move to
    /// \param[in] us Duration of the move, in microseconds
    /// \param[in] relative Bitfield specifying whether each axis should
    ///                     interpret the new position as absolute or
    ///                     relative.
    void setTargetNew(const Point& target, int32_t dda_interval, int32_t us, uint8_t relative);

    /// Instruct the stepper subsystem to move the machine to the
    /// given position.
    /// \param[in] target Position to move to
    /// \param[in] dda_rate dda steps per second for the master axis
    /// \param[in] relative Bitfield specifying whether each axis should
    ///                     interpret the new position as absolute or
    ///                     relative.
    /// \param[in] distance of the move in mm's
    /// \param[in] feedrate of the move in mm's per second multiplied by 64
    void setTargetNewExt(const Point& target, int32_t dda_rate, uint8_t relative, float distance, int16_t feedrateMult64);

    /// Home one or more axes
    /// \param[in] maximums If true, home in the positive direction
    /// \param[in] axes_enabled Bitfield specifiying which axes to
    ///                         home
    /// \param[in] us_per_step Homing speed, in us per step
    void startHoming(const bool maximums,
                     const uint8_t axes_enabled,
		     uint32_t us_per_step);


    /// Enable/disable the given axis.
    /// \param[in] index Index of the axis to enable or disable
    /// \param[in] enable If true, enable the axis. If false, disable.
    void enableAxis(uint8_t index, bool enable);

    /// Enable/disable the axes listed in the bit mask
    /// Various places in the code do a loop over the range of stepper indices
    /// in order to disable or enable them all.  This is a convenience routine
    /// to avoid code the loop and it buys a small amount of code space savings
    void enableAxes(uint8_t axes, bool enable);

    /// Returns a bit mask for all axes enabled
    uint8_t allAxesEnabled(void);
    
    /// Set digial potentiometer value for the axis
    /// \param[in] index Index of the axis 
    /// \param[in] value desired value for potentiometer (0-127 valid)
    void setAxisPotValue(uint8_t index, uint8_t value);

    /// Get the digital potentiometer for stepper axis
    /// \param[in] index Index of the axis
    uint8_t getAxisPotValue(uint8_t index);

    /// Reset the digital potentiometer for stepper axis to the stored eeprom value
    /// \param[in] index Index of the axis
    void resetAxisPot(uint8_t index);

    /// Toggle segment acceleration on or off
    /// Note this is also off if acceleration variable is not set
    void setSegmentAccelState(bool state);

    /// Change the extruder (tool) currently being used.
    /// Extruders have an offset from the center of the platform, and this is used to switch 
    /// offsets when the tool changes.
    /// \param[in] index of the tool
    void changeToolIndex(uint8_t tool);

    /// return a bit field that contains the state of the endstops
    uint8_t getEndstopStatus();

    /// Enables and disables prime / deprime'ing.  Typically used
    /// when loading / unload filament of if deprime / prime is being handled in
    /// Skeinforge
    void deprimeEnable(bool enable);

    /// Run the stepper slice
    void runSteppersSlice();

    /// Handle the interrupt for the steppers (X/Y/Z/A/B axis)
    void doStepperInterrupt();

    /// Handle the interrupt to extrude material
    void doExtruderInterrupt();

    /// Remove offsets from a position; used to determine distance to move when
    /// clearing the platform
    Point removeOffsets(const Point& position);
};

#endif // STEPPERS_HH_
