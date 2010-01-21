#ifndef MOTHERBOARD_BOARDS_RRMB_V12_HH_
#define MOTHERBOARD_BOARDS_RRMB_V12_HH_

// BOILERPLATE +
// Copyright 2010 Makerbot Industries, LLC
// Author: Adam Mayer

// Board configuration files define pin configurations
// for different motherboards, as well as available
// features.

#include "AvrPort.hh"

// Interval for stepper update in microsections
#define INTERVAL_IN_MICROSECONDS 64

// Power supply control pin
#define HAS_PSU         1
#define PSU_PIN         Pin(PortD,6)

// SD card pins
#define HAS_SD          1
#define SD_WRITE_PIN    Pin(PortB,2)
#define SD_DETECT_PIN   Pin(PortB,3)
#define SD_SELECT_PIN   Pin(PortB,4)

// Slave RS485 UART configuration
#define HAS_SLAVE_UART  1
#define TX_ENABLE_PIN   Pin(PortD,4)
#define RX_ENABLE_PIN   Pin(PortD,5)

#define HAS_COMMAND_QUEUE 1

//x axis pins
#define X_STEP_PIN      Pin(PortD,7)
#define X_DIR_PIN       Pin(PortC,2)
#define X_ENABLE_PIN    Pin(PortC,3)
#define X_MIN_PIN       Pin(PortC,4)
#define X_MAX_PIN       Pin(PortC,5)

//y axis pins
#define Y_STEP_PIN      Pin(PortC,7)
#define Y_DIR_PIN       Pin(PortC,6)
#define Y_ENABLE_PIN    Pin(PortA,7)
#define Y_MIN_PIN       Pin(PortA,6)
#define Y_MAX_PIN       Pin(PortA,5)

//z axis pins
#define Z_STEP_PIN      Pin(PortA,4)
#define Z_DIR_PIN       Pin(PortA,3)
#define Z_ENABLE_PIN    Pin(PortA,2)
#define Z_MIN_PIN       Pin(PortA,1)
#define Z_MAX_PIN       Pin(PortA,0)

#endif // MOTHERBOARD_BOARDS_RRMB_V12_HH_
