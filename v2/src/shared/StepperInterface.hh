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

#ifndef STEPPERINTERFACE_HH_
#define STEPPERINTERFACE_HH_

#include <Pin.hh>

/// The StepperInterface module represents a connection to a single stepper controller.
/// \ingroup SoftwareLibraries
class StepperInterface {
private:
    /// Default constructor
    StepperInterface() {}
    StepperInterface(const Pin& dir,
                    const Pin& step,
                    const Pin& enable,
                    const Pin& max,
                    const Pin& min,
                    uint16_t eeprom_base_in);

        friend class Motherboard;

private:
        /// Initialize the pins for the interface
        /// \param[in] idx Stepper index that this interface refers to (used to look up
        ///                it's settings in the EEPROM)
        void init(uint8_t idx);



        Pin dir_pin;                ///< Pin (output) that the direction line is connected to
        Pin step_pin;               ///< Pin (output) that the step line is connected to
        Pin enable_pin;             ///< Pin (output) that the enable line is connected to
        Pin max_pin;                ///< Pin (input) that the maximum endstop is connected to.
        Pin min_pin;                ///< Pin (input) that the minimum endstop is connected to.
        bool invert_endstops;       ///< True if endstops input polarity is inverted for
                                    ///< this axis.
        bool invert_axis;           ///< True if motions for this axis should be inverted

        uint16_t eeprom_base;       ///< Base address to read EEPROM configuration from

public:
	/// Set the direction for the stepper to move
        /// \param[in] forward True to move the stepper forward, false otherwise.
	void setDirection(bool forward);

	/// Set the value of the step line
        /// \param[in] value True to enable, false to disable. This should be toggled
        ///                  back and fourth to effect stepping.
	void step(bool value);

        /// Enable or disable the stepper motor on this axis
        /// \param[in] True to enable the motor
	void setEnabled(bool enabled);

        /// Check if the maximum endstop has been triggered for this axis.
        /// \return True if the axis has triggered its maximum endstop
	bool isAtMaximum();

        /// Check if the minimum endstop has been triggered for this axis.
        /// \return True if the axis has triggered its minimum endstop
	bool isAtMinimum();
};

#endif // STEPPERINTERFACE_HH_
