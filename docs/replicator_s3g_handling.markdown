# S3G protocol (formerly RepRap Generation 3 Protocol Specification)

## Overview

This document describes the way that the Replicator firmware handles s3g commands. Specifically what payload values are accepted, and how unaccepted values are handled.  Commands that are accepted and processed in a way that is obvious from the s3g spec are not listed.
# Host Query Commands

## 00 - Get version: Query firmware for version information
This command allows the host and firmware to exchange version numbers. It also allows for automated discovery of the firmware. Version numbers will always be stored as a single number, Arduino / Processing style.

Payload

    uint16: Host Version

Response

    uint16: Firmware Version

Replicator Handling

    Payload < 25 : returns firmware version 0x0000 (ie invalid version)
    Payload > 25 : returns motherboard firmware version

## 01 - Init: Initialize firmware to boot state
Initialization consists of:

    * Resetting all axes positions to 0
    * Clearing command buffer

Payload (0 bytes)

Response (0 bytes)

Replicator Handling

   not handled. accepted, but no action taken
 
## 03 - Clear buffer: Empty the command buffer
This command will empty our buffer, and reset all pointers, etc to the beginning of the buffer. If writing to an SD card, it will reset the file pointer back to the beginning of the currently open file. Obviously, it should halt all execution of action commands as well.

Payload (0 bytes)

Response (0 bytes)

Replicator Handling 

   a soft reset is called.  same as cmd 7: abort


## 10 - Tool query: Query a tool for information
This command is for sending a query command to the tool. The host firmware will then pass the query along to the appropriate tool, wait for a response from the tool, and pass the response back to the host. TODO: Does the master handle retries?

Payload

    uint8: Tool index 
    0-N bytes: Payload containing the query command to send to the tool.

Response

    0-N bytes: Response payload from the tool query command, if any.

Replicator Handling
    
    There is no separate "tool", but the replicator upholds this format.  Tool query commands with respond with the expected behaviors (except as noted in the tool section of this doc).

## 12 - Read from EEPROM
Read the specified number of bytes from the given offset in the EEPROM, and return them in a response packet. The maximum read size is 31 bytes.

Payload

    uint16: EEPROM memory offset to begin reading from
    uint8: Number of bytes to read, N.

Response

    N bytes: Data read from the EEPROM

Replicator Handling
    we use the avr eeprom library for eeprom access.  This may or may not handle invalid values for offset and length
    TODO: hanldle invalid values for offset and length.   Currently the bot attemts to process all values.

## 13 - Write to EEPROM
Write the given bytes to the EEPROM, starting at the given offset.

Payload

    uint16: EEPROM memory offset to begin writing to
    uint8: Number of bytes to write
    N bytes: Data to write to EEPROM

Response

    uint8: Number of bytes successfully written to the EEPROM

Replicator Handling
    we use the avr eeprom library for eeprom access.  This may or may not handle invalid values for offset and length
    TODO: hanldle invalid values for offset and length.  Currently the bot attemts to process all values.


## 22 - Extended stop: Stop a subset of systems
Stop the stepper motor motion and/or reset the command buffer.  This differs from the reset and abort commands in that a soft reset of all functions i     s not called

Payload

    uint8: Bitfield indicating which subsystems to shut down. If bit 0 is set, halt all stepper motion. If bit 1 is set, clear the command queue.

Response

    int8: 0 If the command terminated normally, 1 if there was an error

Replicator Handling
    a zero response is always sent.  There is no processed failure case

## 23 - Get motherboard status
Retrieve some status information from the motherboard

Payload (0 bytes)

Response

    uint8: Bitfield containing status information

Replicator Handling
    This command does not exist.  returns RC_CMD_UNSUPPORTED

## 26 - Get communication statistics
Gathers statistics about communication over the tool network. This was intended for use while troubleshooting Gen3/4 machines.

Payload (0 bytes)

Response

    uint32: Packets received from the host network
    uint32: Packets sent over the tool network
    uint32: Number of packets sent over the tool network that were not repsonded to
    uint32: Number of packet retries on the tool network 
    uint32: Number of bytes received over the tool network that were discarded as noise

Replicator Handling
    This command does not exist.  returns RC_CMD_UNSUPPORTED

# Host Buffered Commands

## 129 - Queue point
This queues an absolute point to move to.

_Historical note: This implementation is much more wordy than an incremental solution, which likely impacts processing time and buffer sizes on the resource-constrained firmware_

Payload

    int32: X coordinate, in steps
    int32: Y coordinate, in steps
    int32: Z coordinate, in steps
    uint32: Feedrate, in microseconds between steps on the max delta. (DDA)

Replicator Handling
    Command accepted but no action taken

## 130 - Set position
Reset the current position of the axes to the given values.

Payload

    int32: X position, in steps
    int32: Y position, in steps
    int32: Z position, in steps

Replicator Handling
    
    Replicator assumes that the received values are correct.  TODO: invalid values should not be handled.

## 131 - Find axes minimums: Move specified axes in the negative direction until their limit switch is triggered.
This function will find the minimum position that the hardware can travel to, then stop. Note that all axes are moved syncronously. If one of the axes (Z, for example) should be moved separately, then a seperate command should be sent to move that axis. Note that a minimum endstop is required for each axis that is to be moved.

Payload

    uint8: Axes bitfield. Axes whose bits are set will be moved.
    uint32: Feedrate, in microseconds between steps on the max delta. (DDA)
    uint16: Timeout, in seconds.

Replicator Handling
    
    Replicator assumes that the received values are correct.  TODO: invalid values should not be handled.
    
## 132 - Find axes maximums: Move specified axes in the positive direction until their limit switch is triggered.
This function will find the maximum position that the hardware can travel to, then stop. Note that all axes are moved syncronously. If one of the axes (Z, for example) should be moved separately, then a seperate command should be sent to move that axis. Note that a maximum endstop is required for each axis that is to be moved.

Payload

    uint8: Axes bitfield. Axes whose bits are set will be moved.
    uint32: Feedrate, in microseconds between steps on the max delta. (DDA)
    uint16: Timeout, in seconds.

Replicator Handling
    
    Replicator assumes that the received values are correct.  TODO: invalid values should not be handled.

## 135 - Wait for tool ready: Wait until a tool is ready before proceeding
This command halts machine motion until the specified toolhead reaches a ready state. A tool is ready when it's temperature is within range of the setpoint.

Payload

    uint8: Tool ID of the tool to wait for
    uint16: Delay between query packets sent to the tool, in ms (nominally 100 ms)
    uint16: Timeout before continuing without tool ready, in seconds (nominally 1 minute)

Replicator Handling

    Tool ID is expected to be 0 or 1.  Other values will assume tool 0.
    nominal values are not implemented, all values accepted for delay and timeout. 
    The delay parameter is ignored
    A tool timeout of zero will not wait for the tool to heat.  

## 136 - Tool action command: Send an action command to a tool for execution
This command is for sending an action command to the tool. The host firmware will then pass the query along to the appropriate tool, wait for a response from the tool, and pass the response back to the host. TODO: Does the master handle retries?

Payload

    uint8: Tool ID of the tool to query
    uint8: Action command to send to the tool
    uint8: Length of the tool command payload (N)
    N bytes: Tool command payload, 0-? bytes.

Replicator Handling

    There is no separate "tool", however tool commands will return the expected values (with the exceptions noted in the "tool" section)    

## 137 - Enable/disable axes: Explicitly enable or disable stepper motor controllers
This command is used to explicitly power steppers on or off. Generally, it is used to shut down the steppers after a build to save power and avoid generating excessive heat.

Payload

    uint8: Bitfield codifying the command (see below)

<table>
<tr>
 <th>Bit</th>
 <th>Details</th>
</tr>
<tr>
 <th>7</th>
 <th>If set to 1, enable all selected axes. Otherwise, disable all selected axes.</th>
</tr>
<tr>
 <th>6</th>
 <th>N/A</th>
</tr>
<tr>
 <th>5</th>
 <th>N/A</th>
</tr>
<tr>
 <th>4</th>
 <th>B axis select</th>
</tr>
<tr>
 <th>3</th>
 <th>A axis select</th>
</tr>
<tr>
 <th>2</th>
 <th>Z axis select</th>
</tr>
<tr>
 <th>1</th>
 <th>Y axis select</th>
</tr>
<tr>
 <th>0</th>
 <th>X axis select</th>
</tr>
</table>

Replicator Handling
  The current version of the accelerated firmware ignores this command while printing.  The stepper system enables axes when they are moved

## 139 - Queue extended point
This queues an absolute point to move to.

_Historical note: This implementation is much more wordy than an incremental solution, which likely impacts processing time and buffer sizes on the resource-constrained firmware_

Payload

    int32: X coordinate, in steps
    int32: Y coordinate, in steps
    int32: Z coordinate, in steps
    int32: A coordinate, in steps
    int32: B coordinate, in steps
    uint32: Feedrate, in microseconds between steps on the max delta. (DDA)

Replicator Handling
    
    Replicator assumes that the received values are correct.  TODO: invalid values should not be handled.

## 140 - Set extended position
Reset the current position of the axes to the given values.

Payload

    int32: X position, in steps
    int32: Y position, in steps
    int32: Z position, in steps
    int32: A position, in steps
    int32: B position, in steps

Replicator Handling
    
    Replicator assumes that the received values are correct.  TODO: invalid values should not be handled.

## 141 - Wait for platform ready: Wait until a build platform is ready before proceeding
This command halts machine motion until the specified tool device reaches a ready state. A build platform is ready when it's temperature is within range of the setpoint.

Payload

    uint8: Tool ID of the build platform to wait for
    uint16: Delay between query packets sent to the tool, in ms (nominally 100 ms)
    uint16: Timeout before continuing without tool ready, in seconds (nominally 1 minute)

Replicator Handling

    the bot has no knowledge of nominal values.
    Tool ID is ignored.
    Delay is ignored
    Timeout accepts all values.  A timeout of 0 or small will not wait for the tool to heat

## 142 - Queue extended point, new style
This queues a point to move to.

_Historical note: It differs from old-style point queues (see command 139 et. al.) in that it no longer uses the DDA abstraction and instead specifies the total move time in microseconds. Additionally, each axis can be specified as relative or absolute. If the 'relative' bit is set on an axis, then the motion is considered to be relative; otherwise, it is absolute._

Payload

    int32: X coordinate, in steps
    int32: Y coordinate, in steps
    int32: Z coordinate, in steps
    int32: A coordinate, in steps
    int32: B coordinate, in steps
    uint32: Duration of the movement, in microseconds
    uint8: Axes bitfield to specify which axes are relative. Any axis with a bit set should make a relative movement.

Replicator Handling
    
    Replicator assumes that the received values are correct.  TODO: invalid values should not be handled.

## 145 - Set digital potentiometer value
Set the value of the digital potentiometers that control the voltage reference for the botsteps

Payload

    uint8: Axes bitfield to specify which axes' positions to store. Any axes with a bit set should have it's position stored.
    uint8: value (valid range 0-127), values over max will be capped at max

Replicator Handling
    
    max value is 118.  This is due to the wide tolerance of the resistors on the Mightyboard, which make voltage levels variable.    

## 146 - Set RGB LED value
Set Brightness levels for RGB led strip

Payload

    uint8:  red value (all pix are 0-255)
    uint8:  green
    uint8:  blue
    uint8:  blink rate (0-255 valid)
    uint8:  effect (currently unused)

Replicator Handling

    Replicator cannot provide full RGB coverage.  It has two PWM output channels and ON/OFF options.  Thus two LEDs can be set to a full range of brightness levels and the third one must be on/off

## 147 - Set Beep
Set a buzzer frequency and buzz time

Payload
    uint16: frequency
    uint16: buzz length in ms
    uint8:  effect  (currently unused)

Replicator Handling

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
