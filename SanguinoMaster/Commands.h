#ifndef _COMMANDS_H_
#define _COMMANDS_H_
/**
 *  Sanguino 3rd Generation Firmware (S3G)
 *
 *  Specification for this protocol is located at: 
 *    http://docs.google.com/Doc?id=dd5prwmp_14ggw37mfp
 *  
 *  License: GPLv2
 *  Authors: Marius Kintel, Adam Mayer, and Zach Hoeken
 */

/****************************
 ***      WARNING         ***
 ****************************

 * These values here are also used in the replicatorG driver.
 * If you modify anything here, please ensure that it is updated
 * in the Sanguino3GDriver.java file at:
 * https://replicatorg.googlecode.com/svn/trunk/src/replicatorg/app/drivers
 */

// These are our query commands from the host
#define HOST_CMD_VERSION         0
#define HOST_CMD_INIT            1
#define HOST_CMD_GET_BUFFER_SIZE 2
#define HOST_CMD_CLEAR_BUFFER    3
#define HOST_CMD_GET_POSITION    4
#define HOST_CMD_GET_RANGE       5
#define HOST_CMD_SET_RANGE       6
#define HOST_CMD_ABORT           7
#define HOST_CMD_PAUSE           8
#define HOST_CMD_PROBE           9
#define HOST_CMD_TOOL_QUERY     10
#define HOST_CMD_IS_FINISHED    11
#define HOST_CMD_READ_EEPROM    12
#define HOST_CMD_WRITE_EEPROM   13

// Commands for capturing build to a file on an SD card
#define HOST_CMD_CAPTURE_TO_FILE   14
#define HOST_CMD_END_CAPTURE       15
#define HOST_CMD_PLAYBACK_CAPTURE  16

// Software reset, to workaround the problem in the v1.1 and
// v1.2 motherboards that preclude hardware resets.
#define HOST_CMD_RESET             17

// Retrieve the next filename from the SD card's root
// directory.
#define HOST_CMD_NEXT_FILENAME     18

// Retrieve the value of a debugging register
#define HOST_CMD_GET_DBG_REG       19

// These are our bufferable commands from the host
// #define HOST_CMD_QUEUE_POINT_INC   128  // deprecated
#define HOST_CMD_QUEUE_POINT_ABS   129
#define HOST_CMD_SET_POSITION      130
#define HOST_CMD_FIND_AXES_MINIMUM 131
#define HOST_CMD_FIND_AXES_MAXIMUM 132
#define HOST_CMD_DELAY             133
#define HOST_CMD_CHANGE_TOOL       134
#define HOST_CMD_WAIT_FOR_TOOL     135
#define HOST_CMD_TOOL_COMMAND      136
#define HOST_CMD_ENABLE_AXES       137

// These are our query commands from the host
#define SLAVE_CMD_VERSION                0
#define SLAVE_CMD_INIT                   1
#define SLAVE_CMD_GET_TEMP               2
#define SLAVE_CMD_SET_TEMP               3
#define SLAVE_CMD_SET_MOTOR_1_PWM        4
#define SLAVE_CMD_SET_MOTOR_2_PWM        5
#define SLAVE_CMD_SET_MOTOR_1_RPM        6
#define SLAVE_CMD_SET_MOTOR_2_RPM        7
#define SLAVE_CMD_SET_MOTOR_1_DIR        8
#define SLAVE_CMD_SET_MOTOR_2_DIR        9
#define SLAVE_CMD_TOGGLE_MOTOR_1        10
#define SLAVE_CMD_TOGGLE_MOTOR_2        11
#define SLAVE_CMD_TOGGLE_FAN            12
#define SLAVE_CMD_TOGGLE_VALVE          13
#define SLAVE_CMD_SET_SERVO_1_POS       14
#define SLAVE_CMD_SET_SERVO_2_POS       15
#define SLAVE_CMD_FILAMENT_STATUS       16
#define SLAVE_CMD_GET_MOTOR_1_PWM       17
#define SLAVE_CMD_GET_MOTOR_2_PWM       18
#define SLAVE_CMD_GET_MOTOR_1_RPM       19
#define SLAVE_CMD_GET_MOTOR_2_RPM       20
#define SLAVE_CMD_SELECT_TOOL           21
#define SLAVE_CMD_IS_TOOL_READY         22
#define SLAVE_CMD_PAUSE_UNPAUSE         23
#define SLAVE_CMD_ABORT                 24

#endif // _COMMANDS_H_
