/*
 * Copyright 2010 by Adam Mayer <adam@makerbot.com>
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


#ifndef MOTOR_CONTROLLER_HH_
#define MOTOR_CONTROLLER_HH_

#include "Timeout.hh"

/// The motor controller state machine can control a DC (or for gen3, stepper)
/// extruder motor. It is implemented as a singleton.
/// \ingroup SoftwareLibraries
class MotorController {
public:
        /// Construct a new motor controller.
        MotorController();

private:
        static MotorController motor_controller;

public:
        /// Update the motor controller output. This should be called periodically
        /// to ensure that it doesn't fall behind.
	void update();

        /// Set the motor speed by duty cycle
        /// \param[in] speed Speed to run the motor at, in PWM duty cycle (0 - 255).
	void setSpeed(int speed);

        /// Set the motor speed by specifying a target RPM
        /// \param[in] speed Speed to run the motor at, in RPM
	void setRPMSpeed(uint32_t speed);

        /// Get the motor speed setpoint, in RPM
	uint32_t getRPMSpeed();

        /// Get the motor speed stepoint, in PWM duty cycle
	int getSpeed();

        /// Set the driection to rotate the motor
        /// \param[in] dir true is clockwise?
	void setDir(bool dir);

        /// Enable or disable the motor
        /// \param[in] on true to turn the motor on.
	void setOn(bool on);

        /// Toggle whether the motor is paused or unpaused.
	void pause();

        /// Get a reference to the motor controller.
        /// \return The motor controller
	static MotorController& getController() { return motor_controller; }

        /// Update the motor controller state machine.
	static void runMotorSlice() { getController().update(); }

        /// Reset the motor controller to a default state.
	void reset();

private:
        /// For gen3 extruders that have a stepper attached, load the backoff
        /// parameters from the EEPROM.
	void loadBackoffParameters();

        bool set_with_rpm;      ///< If true, #rpm should be used to set the motor
                                ///< speed instead of #speed.
        bool direction;         ///< Direction to turn the motor. True is cockwise.
        bool on;                ///< True if the motor is on.

        // TODO: Why is this an int?
        int speed;              ///< Speed of the motor, in PWM duty cycle.
        uint32_t rpm;           ///< Speed of the motor, in RPM
        bool paused;            ///< True if the motor is paused.

        bool backoff_enabled;   ///< True if backoff is enabled.

        enum {
		BO_INACTIVE,
		BO_HALT_1,
		BO_REVERSE,
		BO_HALT_2,
		BO_FORWARD,
	} backoff_state;
	Timeout current_operation_timeout;
	Timeout forward_trigger_timeout;
	uint32_t halt_ms;
	uint32_t reverse_ms;
	uint32_t forward_ms;
	uint32_t trigger_ms;
};

#endif // MOTOR_CONTROLLER_HH_
