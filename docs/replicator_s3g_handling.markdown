# Replicator Handling of s3g commands

## Overview

This document describes the way that the Replicator firmware handles s3g commands. Specifically what payload values are accepted, and how unaccepted values are handled.   We only list commands that have behaviors different than those described in the general s3g specification @  https://github.com/makerbot/s3g/blob/master/doc/s3g_protocol.markdown

## Ignored Commands (return "success", but take no action)

### Host Query Commands
* 01 Initialize to Boot State

There is no "Command not supported/recognized" packet response for Tool and Host Action commands.  This is because action commands are placed in a queue and are not processed immediately. The bot returns a "Success"  code for Action commands as long as the buffer has space, and the bot is not printing from SD card.  The following action commands are ignored by the replicator:

### Host Action Commands

### Tool Action Commands

*01 - reset firmware
*06 - Set motor speed  
*10 - Enable / disable motor
*14 - Set servo 1 position
*24 - Abort

## Unhandled Query Commands (return "cmd unknown")

### Host Query Commands
*26 Get Communication Stats

### Tool Query Commands
*17 Get Motor Speed


## Commands With Limited behavior

### Host Query Commands

#### 00 - Get version: Query firmware for version information
    HostVersion < 25 : returns firmware version 0x0000 (ie invalid version)
    HostVersion > 25 : returns motherboard firmware version

 
#### 03 - Clear buffer: Empty the command buffer
    a soft reset is called.  same as abort, and reset

## 10 - Tool query: Query a tool for information
    There is no separate "tool", but the replicator upholds this format.  Tool query commands with respond with the expected behaviors (except as noted in the tool section of this doc).  For example, get toolhead temperature responds with the tool temperature, even though the tool is not technically separate from the motherboard

## 12 - Read from EEPROM
    we use the avr eeprom library for eeprom access.  This may or may not handle invalid values for offset and length
    TODO: hanldle invalid values for offset and length.   Currently the bot attemts to process all values.

## 13 - Write to EEPROM
    we use the avr eeprom library for eeprom access.  This may or may not handle invalid values for offset and length
    TODO: hanldle invalid values for offset and length.  Currently the bot attemts to process all values.


## 22 - Extended stop: Stop a subset of systems
    a zero response is always sent.  There is no processed failure case

# Host Buffered Commands

## 131 - Find axes minimums: Move specified axes in the negative direction until their limit switch is triggered.
    The Replicator does not have minimum endstops for X or Y.  Thus get axes minimum for these axes will result in steppers trying to move past the minimum point. (ie bad things will happen) 
    A and B axes also do not have minimum axes, and this command will simply timeout

## 132 - Find axes maximums: Move specified axes in the positive direction until their limit switch is triggered.
    The Replicator does not have maximum endstops for Z.  Thus get axes maximum for the Z axes will result in the stepper trying to move past the maximum point. (ie bad things will happen) 
    A and B axes also do not have minimum axes, and this command will simply timeout
    
## 135 - Wait for tool ready : Wait until a tool is ready before proceeding
    Tool ID is expected to be 0 or 1.  Other values will assume tool 0.
    nominal timeout values are not implemented, all values accepted for timeout. 
    The delay parameter is ignored
    A tool timeout of zero will not wait for the tool to heat.  

## 136 - Tool action command: Send an action command to a tool for execution
    There is no separate "tool", however tool commands will return the expected values (with the exceptions noted in the "tool" section).  For example, set tool temperature will set the extruder temperature, even though the extruder is controlled locally at the motherboard.

## 137 - Enable/disable axes: Explicitly enable or disable stepper motor controllers
  The current version of the accelerated firmware ignores this command while printing.  The stepper system enables axes when they are moved and disables them on reset

## 139 - Queue extended point
Z clipping is implemented.  If a z_axis position greater than 150 is queued, the replicator will clip movement to 150

## 140 - Set extended position
Z clipping is implemented.  If a z_axis position greater than 150 is queued, the replicator will clip movement to 150

## 141 - Wait for platform ready: Wait until a build platform is ready before proceeding
    the bot has no knowledge of nominal values.
    Tool ID is ignored.
    Delay is ignored
    Timeout accepts all values.  A timeout of 0 or small will not wait for the tool to heat

## 142 - Queue extended point, new style
Z clipping is implemented.  If a z_axis position greater than 150 is queued, the replicator will clip movement to 150

## 145 - Set digital potentiometer value
    max value is 118.  This is due to the wide tolerance of the resistors on the Mightyboard, which make voltage levels variable. values above 118 will be clipped

# 146 - Set RGB LED value
    Replicator cannot provide full RGB coverage.  It has two PWM output channels and ON/OFF options.  Thus two LEDs can be set to a full range of brightness levels and the third one must be on/off

## 147 - Set Beep
    There are occasional timing issues with the handling of this command that result in mangled or missed buzzes.
    frequencies above 4978 are equivalent to full on.

## 149 - Display message to LCD
This command is used to display a message to the LCD board.
The maximum buffer size is limited by the maximum package size.  Thus a full screen cannot be written with one command.
Messages are stored in a buffer and the full buffer is displayed when the "last message in group" flag is 1.
The buffer is also displayed when the clear message flag is 1.  TODO: clean this
The "last message in group" flag must be used for display of multi-packet messages.
Normal popping of the message screen, such as when a print is over, is ignored if the "last message in group" flag has not been received.  This is because the bot thinks it is still waiting for the remainder of a message.

if the "clear message" flag is 1, the message buffer will be cleared and any existing timeout out will be cleared.

If the "wait on button" flag is 1, the message screen will clear after a user button press is received.  The timeout field is still relevant if the button press is never received.

Text will auto-wrap at end of line. \n is recognized as new line start. \r is ignored.

Payload
    uint8: Options bitfield (see below)
    uint8: Horizontal position to display the message at (commonly 0-19)
    uint8: Vertical position to display the message at (commonly 0-3)
    uint8: Timeout, in seconds. If 0, this message will left on the screen
    1+N bytes: Message to write to the screen, in ASCII, terminated with a null character.
<table>
<tr>
 <th>Bit</th>
 <th>Name</th>
</tr>
<tr>
 <td>7</td>
 <td>N/A</td>
</tr>
<tr>
 <td>6</td>
 <td>N/A</td>
</tr>
<tr>
 <td>5</td>
 <td>N/A</td>
</tr>
<tr>
 <td>4</td>
 <td>N/A</td>
</tr>
<tr>
 <td>3</td>
 <td>N/A</td>
</tr>
<tr>
<td>2</td>
 <td>wait for button press</td>
</tr>
<tr>
 <td>1</td>
 <td>last message in group</td>
</tr>
<tr>
 <td>0</td>
 <td>clear existing message</td>
</tr>
</table>

Replicator Handling
    
    Replicator assumes that the received values are correct.  TODO: invalid values should not be handled.

## 151 - Queue Song
Play predefined songs on the piezo buzzer

Payload

    uint8: songID  - select from a predefined list of songs

Replicator Handling

    songID 0:  error tone with 4 cycles
    songID 1:  done tone
    songID 2:  error tone with 2 cycles 
    all other songIDs set to song ID 2

## 153 - Build start notification
Tells the motherboard that a build is about to begin, and provides the name of the job for status reporting. This allows the motherboard to display an appropriate build screen on the interface board.

Payload

    uint32: Number of steps (commands?) in the build
    1+N bytes: Name of the build, in ASCII, null terminated

Response (0 bytes)

Replicator Handling

    build steps is ignored


# Tool Query Commands

## 00 - Get version: Query firmware for version information
This command allows the host and firmware to exchange version numbers. It also allows for automated discovery of the firmware. Version numbers will always be stored as a single number, Arduino / Processing style.

Payload

    uint16: Host Version

Response

    uint16: Firmware Version

Replicator Handling

    motherboard firmware version is returned

## 17 - Get motor speed (RPM)

Payload (0 bytes)

Response

    uint32: Duration of each rotation, in microseconds

Replicator Handling

    not handled.  returns RC_COMMAND_UNSUPPORTED

## 25 - Read from EEPROM
Read the specified number of bytes from the given offset in the EEPROM, and return them in a response packet. The maximum read size is 31 bytes.

Payload

    uint16: EEPROM memory offset to begin reading from
    uint8: Number of bytes to read, N.

Response

    N bytes: Data read from the EEPROM

Replicator Handling

    reads from motherboard eeprom.  note that the eeprom maps are not the same between 5.x and previous firmware versions.

## 26 - Write to EEPROM
Write the given bytes to the EEPROM, starting at the given offset.

Payload

    uint16: EEPROM memory offset to begin writing to
    uint8: Number of bytes to write
    N bytes: Data to write to EEPROM

Response

    uint8: Number of bytes successfully written to the EEPROM

Replicator Handling

    reads from motherboard eeprom.  note that the eeprom maps are not the same between 5.x and previous firmware versions.

# Tool Action Commands

## 01 - Init: Initialize firmware to boot state
Initialization consists of:

    * Resetting target temperatures to 0
    * Turning off all outputs (fan, motor, etc)
    * Detaching all servo devices
    * Resetting motor speed to 0

Payload (0 bytes)

Response (0 bytes)

Replicator Handling

    resets motherboard

## 03 - Set toolhead target temperature
This sets the desired temperature for the heating element. The tool firmware will then attempt to maintain this temperature as closely as possible.

Payload

    int16: Desired target temperature, in Celsius

Response (0 bytes)

Replicator Handling

    values less than 0 will be set to zero
    values greater than max temp (currently 280) will be set to max temp

## 06 - Set motor speed (RPM)
This sets the motor speed as an RPM value, but does not enable/disable it.

Payload

    uint32: Duration of each rotation, in microseconds

Response (0 bytes)

Replicator Handling

    ignored

## 10 - Enable/disable motor
This command can be used to turn the motor on or off. The motor direction must be specified when enabling the motor.

Payload

    uint8: Bitfield codifying the command (see below)

Response (0 bytes)

<table>
<tr>
 <th>Bit</th>
 <th>Name</th>
 <th>Details</th>
</tr>
<tr>
 <th>7</th>
 <th>N/A</th>
 <th></th>
</tr>
<tr>
 <th>6</th>
 <th>N/A</th>
 <th></th>
</tr>
<tr>
 <th>5</th>
 <th>N/A</th>
 <th></th>
</tr>
<tr>
 <th>4</th>
 <th>N/A</th>
 <th></th>
</tr>
<tr>
 <th>3</th>
 <th>N/A</th>
 <th></th>
</tr>
<tr>
 <th>2</th>
 <th>N/A</th>
 <th></th>
</tr>
<tr>
 <th>1</th>
 <th>DIR</th>
 <th>If set, motor should be turned in a clockwise direciton. Otherwise, it should be turned in a counterclockwise direction</th>
</tr>
<tr>
 <th>0</th>
 <th>ENABLE</th>
 <th>If set, enable the motor. If unset, disable the motor</th>
</tr>
</table>

Replicator Handling

    ignored

## 14 - Set servo 1 position
Set the position of a servo connected to the first servo output.

Payload

    uint8: Desired angle, from 0 - 180

Response (0 bytes)

Replicator Handling

    This command is not handled.  TODO: should it be ignored?


## 23 - Pause/resume: Halt execution temporarily
This function is inteded to be called infrequently by the end user in order to make build-time adjustments during a print.

Payload (0 bytes)

Response (0 bytes)

Replicator Handling

    motherboard command buffer paused

## 24 - Abort immediately: Terminate all operations and reset
This function is intended to be used to terminate a print during printing. Disables any engaged heaters and motors. 

Payload (0 bytes)

Response (0 bytes)

Replicator Handling

    This command is not handled.  TODO: should it be ignored?

## 31 - Set build platform target temperature
This sets the desired temperature for the build platform. The tool firmware will then attempt to maintain this temperature as closely as possible.

Payload

    int16: Desired target temperature, in Celsius

Response (0 bytes)

Replicator Handling

    values less than 0 will be set to zero
    values greater than max temp (currently 280) will be set to max temp

## 38 - Set motor speed (DDA)
This sets the motor speed as a DDA value, in microseconds between step. It should not actually enable the motor until the motor enable command is given. For future implementation of 5D (vs 4D) two DDA codes are sent - the DDA to start with and the DDA to end with. The third uint32 is the number of steps to take. The direction to go is set by code 8, 'Set motor direction'

Payload

    uint32: Speed, in microseconds between steps (start of movement)
    uint32: Speed, in microseconds between steps (end of movement)
    uint32: total steps to take.

Response (0 bytes)

Replicator Handling

    ignored.

## 40 - Light indicator LED
This command turns on an indicator light (for gen 4, the motor direction LED). This command is intended to serve as visual feedback to an operator that the electronics are communicating properly. Note that it should not be used during regular operation, because it interferes with h-bridge operation.

Payload (0 bytes)

Response (0 bytes)

Replicator Handling

    This command is not handled.  TODO: should it be ignored?
