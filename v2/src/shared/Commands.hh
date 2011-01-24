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

#ifndef SHARED_COMMANDS_H_
#define SHARED_COMMANDS_H_

/**
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
// Retrieve the string representing this build
#define HOST_CMD_GET_BUILD_NAME    20

#define HOST_CMD_GET_POSITION_EXT  21
#define HOST_CMD_EXTENDED_STOP     22

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
#define HOST_CMD_WAIT_FOR_PLATFORM 141

#define HOST_CMD_QUEUE_POINT_EXT   139
#define HOST_CMD_SET_POSITION_EXT  140

#define HOST_CMD_QUEUE_POINT_NEW   142

#define HOST_CMD_DEBUG_ECHO        0x70

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
#define SLAVE_CMD_READ_FROM_EEPROM      25
#define SLAVE_CMD_WRITE_TO_EEPROM       26

#define SLAVE_CMD_GET_PLATFORM_TEMP     30
#define SLAVE_CMD_SET_PLATFORM_TEMP     31
#define SLAVE_CMD_GET_SP                32
#define SLAVE_CMD_GET_PLATFORM_SP       33
// Retrieve the string representing this build
#define SLAVE_CMD_GET_BUILD_NAME        34
#define SLAVE_CMD_IS_PLATFORM_READY     35
#define SLAVE_CMD_GET_TOOL_STATUS       36
#define SLAVE_CMD_GET_PID_STATE         37
#endif // SHARED_COMMANDS_H_
