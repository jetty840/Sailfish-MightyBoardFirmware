/****************************************************************************************
 * Here's where you define the overall electronics setup for your machine.
 ****************************************************************************************/

// This  is -*- c++ -*-, no kidding
// define the parameters of our machine.
#define X_STEPS_PER_INCH 416.772354
#define X_STEPS_PER_MM   11.7892
#define X_MOTOR_STEPS    400

#define Y_STEPS_PER_INCH 416.772354
#define Y_STEPS_PER_MM   11.0716
#define Y_MOTOR_STEPS    400

#define Z_STEPS_PER_INCH 16256.0
#define Z_STEPS_PER_MM   320.0
#define Z_MOTOR_STEPS    400

//our maximum feedrates
#define FAST_XY_FEEDRATE 2500.0
#define FAST_Z_FEEDRATE  1000.0

// Units in curve section
#define CURVE_SECTION_INCHES 0.019685
#define CURVE_SECTION_MM 0.5

// Set to one if endstop outputs are inverting (ie: 1 means open, 0 means closed)
#define ENDSTOPS_INVERTING 1

// The *_ENABLE_PIN signals are active high as default. Define this
// to one if they should be active low instead (e.g. if you're using different
// stepper boards).
#define INVERT_ENABLE_PINS 1

// If you use this firmware on a cartesian platform where the
// stepper direction pins are inverted, set these defines to 1
// for the axes which should be inverted.
// RepRap stepper boards are *not* inverting.
#define INVERT_X_DIR 0
#define INVERT_Y_DIR 0
#define INVERT_Z_DIR 0

//
// CHOOSE WHICH MOTHERBOARD YOU'RE USING:
//
#define REPRAP_MOTHERBOARD_VERSION_1_0
//#define REPRAP_MOTHERBOARD_VERSION_1_1

//
// CHOOSE WHICH FAMILY OF STEPPER DRIVER YOU'RE USING:
//
//#define STEPPER_DRIVER_VERSION_1_X
#define STEPPER_DRIVER_VERSION_2_X

//
// CHOOSE WHICH FAMILY OF OPTO ENDSTOP YOU'RE USING:
//
//#define OPTO_ENDSTOP_1_X
#define OPTO_ENDSTOP_2_X

#define PACKET_TIMEOUT 500
#define HOST_SERIAL_SPEED 19200
#define SLAVE_SERIAL_SPEED 38400

//uncomment to enable debugging functions
#define ENABLE_DEBUG 1
#define ENABLE_COMMS_DEBUG 1

//#define SCAN_TOOLS_ON_STARTUP

/****************************************************************************************
 * Sanguino Pin Assignment
 ****************************************************************************************/

//these are the pins for the v1.0 Motherboard.
#ifdef REPRAP_MOTHERBOARD_VERSION_1_0

//x axis pins
#define X_STEP_PIN      15
#define X_DIR_PIN       18
#define X_ENABLE_PIN    19
#define X_MIN_PIN       20
#define X_MAX_PIN       21

//y axis pins
#define Y_STEP_PIN      23
#define Y_DIR_PIN       22
#define Y_ENABLE_PIN    19
#define Y_MIN_PIN       25
#define Y_MAX_PIN       26

//z axis pins
#define Z_STEP_PIN      29
#define Z_DIR_PIN       30
#define Z_ENABLE_PIN    31
#define Z_MIN_PIN        1
#define Z_MAX_PIN        2

//our pin for debugging.
#define DEBUG_PIN        0

//our SD card pins
#define SD_CARD_SELECT	4
#define SD_CARD_WRITE	28
#define SD_CARD_DETECT	24


//our RS485 pins
#define RX_ENABLE_PIN	13
#define TX_ENABLE_PIN	12


#endif

//these are the pins for the v1.1 Motherboard.
#ifdef REPRAP_MOTHERBOARD_VERSION_1_1

//x axis pins
#define X_STEP_PIN      15
#define X_DIR_PIN       18
#define X_ENABLE_PIN    19
#define X_MIN_PIN       20
#define X_MAX_PIN       21

//y axis pins
#define Y_STEP_PIN      23
#define Y_DIR_PIN       22
#define Y_ENABLE_PIN    24
#define Y_MIN_PIN       25
#define Y_MAX_PIN       26

//z axis pins
#define Z_STEP_PIN      27
#define Z_DIR_PIN       28
#define Z_ENABLE_PIN    29
#define Z_MIN_PIN       30
#define Z_MAX_PIN       31

//our pin for debugging.
#define DEBUG_PIN        0

//various SPI select pins
#define SPI_SELECT_1     1
#define SPI_SELECT_2     3
#define SPI_SELECT_3    14

//our SD card pins
#define SD_CARD_SELECT   4
#define SD_CARD_WRITE    2
#define SD_CARD_DETECT   3

//our RS485 pins
#define RX_ENABLE_PIN	13
#define TX_ENABLE_PIN	12

#endif

/****************************************************************************************
 * Stepper Driver Behaviour Definition
 ****************************************************************************************/

//do we want a step delay (ie: length of pulse in microseconds) comment out to disable.
#define STEP_DELAY 5

#ifdef STEPPER_DRIVER_VERSION_1_X
#define STEPPER_ENABLE    1
#define STEPPER_DISABLE   0
#endif

#ifdef STEPPER_DRIVER_VERSION_2_X
#define STEPPER_ENABLE    0
#define STEPPER_DISABLE   1
#endif

/****************************************************************************************
 * Opto Endstop Behaviour Definition
 ****************************************************************************************/

#ifdef OPTO_ENDSTOP_1_X
#define SENSORS_INVERTING 0
#endif

#ifdef OPTO_ENDSTOP_2_X
#define SENSORS_INVERTING 1
#endif


/****************************************************************************************
 * Various Buffer Size Declarations
 ****************************************************************************************/
//we store all queueable commands in one big giant buffer.
// Explicitly allocate memory at compile time for buffer.
#define COMMAND_BUFFER_SIZE 2048
#define POINT_QUEUE_SIZE 32
#define POINT_SIZE 9
#define MAX_PACKET_LENGTH 32
