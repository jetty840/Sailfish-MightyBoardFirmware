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

// Interval for timer update in microseconds
// Servos are locked to this, so this must be 2500.
// (It turns out that nothing needed microsecond timing, anyway.)
#define INTERVAL_IN_MICROSECONDS 2500

// Power supply control pin
#define HAS_PSU         0

// SD card pins
#define HAS_SD          0

// Fix for buggy RS485 chips: no known Gen3 hardware has this issue.
#undef ASSERT_LINE_FIX

// Host RS485 UART configuration
#define HAS_SLAVE_UART  0
#define TX_ENABLE_PIN   Pin(PortC,2)
#define RX_ENABLE_PIN   Pin(PortD,4)

// Extrusion head heater configuration
#define HAS_HEATER      1

#define HAS_SERVOS		1

#define HAS_THERMISTOR  1
// Extruder thermistor analog pin
#define THERMISTOR_PIN  3

// Heated platform configuration
#define HAS_HEATED_PLATFORM		0
// Platform thermistor analog pin
#define PLATFORM_PIN    6

#define CHANNEL_A				Pin(PortC,1)
#define CHANNEL_B				Pin(PortB,3)
#define CHANNEL_C				Pin(PortB,4)

// Fan configuration
#define HAS_FAN				1
#define FAN_ENABLE_PIN		CHANNEL_C

#define HB1_ENABLE_PIN		Pin(PortD,5)
#define HB1_DIR_PIN			Pin(PortD,7)

#define HB2_ENABLE_PIN		Pin(PortD,6)
#define HB2_DIR_PIN			Pin(PortB,0)

// define the tick length, or how often the interrupt is called,
// for the external stepper, in half microseconds:
// 200 means 100 us -> 10 KHz
#define ES_TICK_LENGTH 200

// Enable = "D10"
#define ES_ENABLE_PIN		Pin(PortB,2)
// Dir and step are in the quadrature
#define ES_DIR_PIN			Pin(PortD,2) // Quadrature pin 8
#define ES_STEP_PIN			Pin(PortD,3) // Quadrature pin 7

#define DEBUG_LED			Pin(PortB,5)

#define SAMPLE_INTERVAL_MICROS_THERMISTOR (50L * 1000L)

#endif // BOARDS_ECV22_CONFIGURATION_HH_
