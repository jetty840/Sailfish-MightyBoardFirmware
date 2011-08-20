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

#ifndef BOARDS_RRMBV12_CONFIGURATION_HH_
#define BOARDS_RRMBV12_CONFIGURATION_HH_

// This file details the pin assignments and features of the RepRap Motherboard
// version 1.2 for the ordinary use case.


// Interval for the stepper update in microseconds.  This interval is the minimum
// possible time between steps; in practical terms, your time between steps should
// be at least eight times this large.  Reducing the interval can cause resource
// starvation; leave this at 64uS or greater unless you know what you're doing.
#define INTERVAL_IN_MICROSECONDS 64

// The pin that connects to the /PS_ON pin on the PSU header.  This pin switches
// on the PSU when pulled low.
#define PSU_PIN                 Pin(PortD,6)

// --- Secure Digital Card configuration ---
// NOTE: If SD support is enabled, it is implicitly assumed that the
// following pins are connected:
//  AVR    |   SD header
//---------|--------------
//  MISO   |   DATA_OUT
//  MOSI   |   DATA_IN
//  SCK    |   CLK

// Define as 1 if and SD card slot is present; 0 if not.
#define HAS_SD                  1
// The pin that connects to the write protect line on the SD header.
#define SD_WRITE_PIN            Pin(PortB,2)
// The pin that connects to the card detect line on the SD header.
#define SD_DETECT_PIN           Pin(PortB,3)
// The pin that connects to the chip select line on the SD header.
#define SD_SELECT_PIN           Pin(PortB,4)

// --- Slave UART configuration ---
// The slave UART is presumed to be an RS485 connection through a sn75176 chip.
// Define as 1 if the slave UART is present; 0 if not.
#define HAS_SLAVE_UART          1
// The pin that connects to the driver enable line on the RS485 chip.
#define TX_ENABLE_PIN           Pin(PortD,4)
// The pin that connects to the active-low recieve enable line on the RS485 chip.
#define RX_ENABLE_PIN           Pin(PortD,5)

// --- Host UART configuration ---
// The host UART is presumed to always be present on the RX/TX lines.

// --- Axis configuration ---
// Define the number of stepper axes supported by the board.  The axes are
// denoted by X, Y, Z, A and B.
#define STEPPER_COUNT           3

// --- Stepper and endstop configuration ---
// Pins should be defined for each axis present on the board.  They are denoted
// X, Y, Z, A and B respectively.

// This indicates the default interpretation of the endstop values.
// If your endstops are based on the H21LOB, they are inverted;
// if they are based on the H21LOI, they are not.
#define DEFAULT_INVERTED_ENDSTOPS 1

// The X stepper step pin (active on rising edge)
#define X_STEP_PIN              Pin(PortD,7)
// The X direction pin (forward on logic high)
#define X_DIR_PIN               Pin(PortC,2)
// The X stepper enable pin (active low)
#define X_ENABLE_PIN            Pin(PortC,3)
// The X minimum endstop pin (active high)
#define X_MIN_PIN               Pin(PortC,4)
// The X maximum endstop pin (active high)
#define X_MAX_PIN               Pin(PortC,5)

// The Y stepper step pin (active on rising edge)
#define Y_STEP_PIN              Pin(PortC,7)
// The Y direction pin (forward on logic high)
#define Y_DIR_PIN               Pin(PortC,6)
// The Y stepper enable pin (active low)
#define Y_ENABLE_PIN            Pin(PortA,7)
// The Y minimum endstop pin (active high)
#define Y_MIN_PIN               Pin(PortA,6)
// The Y maximum endstop pin (active high)
#define Y_MAX_PIN               Pin(PortA,5)

// The Z stepper step pin (active on rising edge)
#define Z_STEP_PIN              Pin(PortA,4)
// The Z direction pin (forward on logic high)
#define Z_DIR_PIN               Pin(PortA,3)
// The Z stepper enable pin (active low)
#define Z_ENABLE_PIN            Pin(PortA,2)
// The Z minimum endstop pin (active high)
#define Z_MIN_PIN               Pin(PortA,1)
// The Z maximum endstop pin (active high)
#define Z_MAX_PIN               Pin(PortA,0)

// --- Debugging configuration ---
// The pin which controls the debug LED (active high)
#define DEBUG_PIN               Pin(PortB,0)

// By default, debugging packets should be honored; this is made
// configurable if we're short on cycles or EEPROM.
// Define as 1 if debugging packets are honored; 0 if not.
#define HONOR_DEBUG_PACKETS     1

#endif // BOARDS_RRMBV12_CONFIGURATION_HH_
