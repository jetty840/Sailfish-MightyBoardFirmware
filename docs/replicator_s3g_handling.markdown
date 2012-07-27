# Replicator Handling of s3g commands

## Overview

This document describes the way that the Replicator firmware handles s3g commands. Specifically what payload values are accepted, and how unaccepted values are handled.   We only list commands that have behaviors different than those described in the general s3g specification @  https://github.com/makerbot/s3g/blob/master/doc/s3g_protocol.markdown

## Packet Errors
Currently, the Replicator does not return error codes for invalid packets, instead the packet process times out.  If the packet has an invalid CRC, invalid length, etc.  The packet will not be processed and the s3g host will timeout waiting for a response.  
The Replicator does return special response codes in the following cases (these are not related to packet communication errors)

* Build_Cancel : user has canceled an active host build
* Active_Build : bot is  building from SD card, or running an onboard script, no action commands can be processed
* CMD_Unsupported : bot does not understand the received command

## Ignored Commands (return "success", but take no action)

### Host Query Commands
* 01 Initialize to Boot State

There is no "Command not supported/recognized" packet response for Tool and Host Action commands.  This is because action commands are placed in a queue and are not processed immediately. The bot returns a "Success"  code for Action commands as long as the buffer has space, and the bot is not printing from SD card.  The following action commands are ignored by the replicator:

### Host Action Commands
* 133 - Delay

### Tool Action Commands

*01 - init: reset firmware
*06 - Set motor speed  
*10 - Enable / disable motor
*14 - Set servo 1 position
*24 - Abort

## Unhandled Query Commands (return "cmd unknown")

### Host Query Commands
*26 Get Communication Stats

### Tool Query Commands
*17 Get Motor Speed
*25 Read from EEPROM
*26 Write to EEPROM

## Commands With Limited or specialized behavior

### Host Query Commands

#### 00 - Get version: Query firmware for version information
    HostVersion < 25 : returns firmware version 0x0000 (ie invalid version)
    HostVersion > 25 : returns motherboard firmware version

 
#### 03 - Clear buffer: Empty the command buffer
    a soft reset is called.  same as abort, and reset

#### 10 - Tool query: Query a tool for information
    There is no separate "tool", but the replicator upholds this format.  Tool query commands with respond with the expected behaviors (except as noted in the tool section of this doc).  For example, get toolhead temperature responds with the tool temperature, even though the tool is not technically separate from the motherboard

#### 12 - Read from EEPROM
    we use the avr eeprom library for eeprom access.  This may or may not handle invalid values for offset and length
    TODO: hanldle invalid values for offset and length.   Currently the bot attemts to process all values.

#### 13 - Write to EEPROM
    we use the avr eeprom library for eeprom access.  This may or may not handle invalid values for offset and length
    TODO: hanldle invalid values for offset and length.  Currently the bot attemts to process all values.


#### 22 - Extended stop: Stop a subset of systems
    a zero response is always sent.  There is no processed failure case

### Host Buffered Commands

#### 131 - Find axes minimums: Move specified axes in the negative direction until their limit switch is triggered.
    The Replicator does not have minimum endstops for X or Y.  Thus get axes minimum for these axes will result in steppers trying to move past the minimum point. (ie bad things will happen) 
    A and B axes also do not have minimum axes, and this command will simply timeout

#### 132 - Find axes maximums: Move specified axes in the positive direction until their limit switch is triggered.
    The Replicator does not have maximum endstops for Z.  Thus get axes maximum for the Z axes will result in the stepper trying to move past the maximum point. (ie bad things will happen) 
    A and B axes also do not have minimum axes, and this command will simply timeout
    
#### 135 - Wait for tool ready : Wait until a tool is ready before proceeding
    Tool ID is expected to be 0 or 1.  Other values will assume tool 0.
    nominal timeout values are not implemented, all values accepted for timeout. 
    The delay parameter is ignored
    A tool timeout of zero will not wait for the tool to heat.  

#### 136 - Tool action command: Send an action command to a tool for execution
    There is no separate "tool", however tool commands will return the expected values (with the exceptions noted in the "tool" section).  For example, set tool temperature will set the extruder temperature, even though the extruder is controlled locally at the motherboard.

#### 137 - Enable/disable axes: Explicitly enable or disable stepper motor controllers
  The current version of the accelerated firmware ignores this command while printing.  The stepper system enables axes when they are moved and disables them on reset

#### 139 - Queue extended point
Z clipping is implemented.  If a z_axis position greater than 150 is queued, the replicator will clip movement to 150

#### 140 - Set extended position
Z clipping is implemented.  If a z_axis position greater than 150 is queued, the replicator will clip movement to 150

#### 141 - Wait for platform ready: Wait until a build platform is ready before proceeding
    the bot has no knowledge of nominal values.
    Tool ID is ignored.
    Delay is ignored
    Timeout accepts all values.  A timeout of 0 or small will not wait for the tool to heat

#### 142 - Queue extended point, new style
Z clipping is implemented.  If a z_axis position greater than 150 is queued, the replicator will clip movement to 150

#### 145 - Set digital potentiometer value
    max value is 118.  This is due to the wide tolerance of the resistors on the Mightyboard, which make voltage levels variable. values above 118 will be clipped

#### 146 - Set RGB LED value
    Replicator cannot provide full RGB coverage.  It has two PWM output channels and ON/OFF options.  Thus two LEDs can be set to a full range of brightness levels and the third one must be on/off

#### 147 - Set Beep
    There are occasional timing issues with the handling of this command that result in mangled or missed buzzes.
    frequencies above 4978 are equivalent to full on.

#### 151 - Queue Song
    songIDs > 2 are set to song ID 2

### Tool Query Commands

#### 00 - Get version: Query firmware for version information
    motherboard firmware version is returned

#### 25 - Read from EEPROM
    reads from motherboard eeprom.  note that the eeprom maps are not the same between 5.x and previous firmware versions.

#### 26 - Write to EEPROM
    writes to  motherboard eeprom.  note that the eeprom maps are not the same between 5.x and previous firmware versions.

# Tool Action Commands

#### 03 - Set toolhead target temperature
    values less than 0 will be set to zero
    values greater than max temp (currently 280) will be set to max temp

#### 31 - Set build platform target temperature
    values less than 0 will be set to zero
    values greater than max temp (currently 280) will be set to max temp

