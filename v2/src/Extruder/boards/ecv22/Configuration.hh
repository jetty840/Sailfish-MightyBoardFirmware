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

#ifndef BOARDS_ECV22_CONFIGURATION_HH_
#define BOARDS_ECV22_CONFIGURATION_HH_

// Board configuration files define pin configurations
// for different motherboards, as well as available
// features.

#include "AvrPort.hh"

#define DEVICE_ID		0

// Interval for stepper update in microsections
#define INTERVAL_IN_MICROSECONDS 64

// Power supply control pin
#define HAS_PSU         0

// SD card pins
#define HAS_SD          0

// Host RS485 UART configuration
#define HAS_SLAVE_UART  0
#define TX_ENABLE_PIN   Pin(PortC,2)
#define RX_ENABLE_PIN   Pin(PortD,4)

// Extrusion head heater configuration
#define HAS_HEATER      1

// Heated platform configuration
#define HAS_HEATED_PLATFORM		0

#define HAS_THERMISTOR  1
#define THERMISTOR_PIN  3
#define PLATFORM_PIN    6

#define HAS_DC_MOTOR      1
#define MOTOR_ENABLE_PIN  Pin(PortD,5)
#define MOTOR_DIR_PIN     Pin(PortD,7)


#endif // BOARDS_ECV22_CONFIGURATION_HH_
