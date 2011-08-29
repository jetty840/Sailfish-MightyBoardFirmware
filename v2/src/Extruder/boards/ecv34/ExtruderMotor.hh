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

#ifndef BOARDS_ECV34_EXTRUDER_MOTOR_HH_
#define BOARDS_ECV34_EXTRUDER_MOTOR_HH_

#include <stdint.h>

/// Initialize the extruder motor
/// \ingroup ECv34
void initExtruderMotor();

/// Turn the extruder motor on
/// \param[in] speed Set the motor speed, -255 to 255, where 0 means stop.
/// \ingroup ECv34
void setExtruderMotor(int16_t speed);

/// For extruders with stepper motors: Set the stepper mode.
/// \param[in] mode true to enable stepper, or false to disable.
/// \param[in] external true if the stepper driver is external.
/// \ingroup ECv34
void setStepperMode(bool mode, bool external = false);

/// For extruders with stepper motors:
/// \param[in] micros stepper speed, in microseconds per pulse
/// \param[in] direction true = clockwise
/// \ingroup ECv34
void setExtruderMotorRPM(uint32_t micros, bool direction);

#endif // BOARDS_ECV34_EXTRUDER_MOTOR_HH_
