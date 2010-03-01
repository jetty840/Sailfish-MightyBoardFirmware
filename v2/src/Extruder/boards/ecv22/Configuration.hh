#ifndef EXTRUDER_BOARDS_EC_V22_HH_
#define EXTRUDER_BOARDS_EC_V22_HH_

// BOILERPLATE +
// Copyright 2010 Makerbot Industries, LLC
// Author: Adam Mayer

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

#define HAS_DC_MOTOR      1
#define MOTOR_ENABLE_PIN  Pin(PortD,5)
#define MOTOR_DIR_PIN     Pin(PortD,7)


#endif // EXTRUDER_BOARDS_EC_V22_HH_
