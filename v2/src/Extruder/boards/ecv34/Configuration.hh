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

#ifndef BOARDS_ECV34_CONFIGURATION_HH_
#define BOARDS_ECV34_CONFIGURATION_HH_

// Board configuration files define pin configurations
// for different motherboards, as well as available
// features.

#include "AvrPort.hh"

#define DEVICE_ID		0

// Interval for timer update in microseconds
#define INTERVAL_IN_MICROSECONDS 64

// SD card pins
#define HAS_SD          0

// Fix for buggy RS485 chips
#define ASSERT_LINE_FIX 1

// Host RS485 UART configuration
#define HAS_SLAVE_UART  0
#define TX_ENABLE_PIN   Pin(PortC,0)
#define RX_ENABLE_PIN   Pin(PortC,1)

// Extrusion head heater configuration
#define HAS_HEATER      1

#define HAS_SERVOS		0

#define HAS_THERMISTOR  0

#define HAS_THERMOCOUPLE 1
// Extruder thermistor analog pin
#define THERMOCOUPLE_CS 	Pin(PortD,4)
#define THERMOCOUPLE_SCK	Pin(PortB,5)
#define THERMOCOUPLE_SO		Pin(PortB,4)

// Heated platform configuration
#define HAS_HEATED_PLATFORM		0
// Platform thermistor analog input
#define PLATFORM_PIN    6

#define HAS_SERVOS		1
#define SERVO0			Pin(PortC,2)
#define SERVO1			Pin(PortC,3)

#define CHANNEL_A				Pin(PortB,2)
#define CHANNEL_B				Pin(PortB,1)
#define CHANNEL_C				Pin(PortD,6)

#define HAS_DC_MOTOR		1
#define MOTOR_ENABLE_PIN	Pin(PortD,5)
#define MOTOR_DIR_PIN		Pin(PortB,0)

// We are disabling the debug LED to avoid stomping on SCK.
// #define DEBUG_LED			Pin(PortB,5)

#define SAMPLE_INTERVAL_MICROS_THERMISTOR (50L * 1000L)
#define SAMPLE_INTERVAL_MICROS_THERMOCOUPLE (500L * 1000L)

#endif // BOARDS_ECV34_CONFIGURATION_HH_
