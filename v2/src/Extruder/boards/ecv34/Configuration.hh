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

// TODO: Ditch this in favor of a unified board.hh, or something.
#define IS_EXTRUDER_BOARD

// Interval for timer update in microseconds
#define INTERVAL_IN_MICROSECONDS 64

// SD card pins
#define HAS_SD          0

// Fix for buggy RS485 chips
#define ASSERT_LINE_FIX 1

// Host RS485 UART configuration
#define HAS_SLAVE_UART  0

extern Pin TX_ENABLE_PIN;
extern Pin RX_ENABLE_PIN;

// Extrusion head heater configuration
#define HAS_HEATER      1

/// True if there are any thermistors on the board
#define HAS_THERMISTOR_TABLES

#define HAS_THERMISTOR  0

#define HAS_THERMOCOUPLE 1

// Extruder thermistor analog pin
extern Pin THERMOCOUPLE_CS;
extern Pin THERMOCOUPLE_SCK;
extern Pin THERMOCOUPLE_SO;

// Heated platform configuration
#define HAS_HEATED_PLATFORM		0
// Platform thermistor analog input
#define PLATFORM_PIN    6

#define HAS_SERVOS		1
extern Pin SERVO0;
extern Pin SERVO1;

extern Pin CHANNEL_A;
extern Pin CHANNEL_B;
extern Pin CHANNEL_C;

#define HAS_DC_MOTOR		1

extern Pin MOTOR_ENABLE_PIN;
extern Pin MOTOR_DIR_PIN;

// We are disabling the debug LED to avoid stomping on SCK.
// #define DEBUG_LED			Pin(PortB,5)

#define SAMPLE_INTERVAL_MICROS_THERMISTOR (50L * 1000L)
#define SAMPLE_INTERVAL_MICROS_THERMOCOUPLE (500L * 1000L)

#endif // BOARDS_ECV34_CONFIGURATION_HH_
