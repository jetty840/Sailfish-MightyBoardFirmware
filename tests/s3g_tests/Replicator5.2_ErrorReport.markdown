#Error Report

## Makerbot Test Explanations

### CommonFunctionTests
This suite tests any shared functions contained within Makerbottests.py

### s3gPacketTests
This suite tests the packets that are sent to the machine, and ensures the proper errors (if any) are thrown if a malformed packet is sent to a bot

### s3gSendReceiveTests
This suite tests all functions of the s3g python module, and ensures that all packets can be sent to the machine, and that the machine sends the proper response back.  These tests are pretty simple, and only involve executing the command, and asserting true if no errors are thrown (any errors that arrise will prohibit the assertion from being executed)

### s3gFunctionTests
These tests ensure that the machine processes the command correctly and takes the correct action

### s3gSDCardTests
This suite tests the machine operations that revolve around the SD card.  They are separate from the s3gFunctionTests because they require an SD card to be inserted into the bot.  The SD card in the bot should have the same contents of testFiles


##Commands that should be deprecated
There are several commands that should be deprecated and not understood by the replicator.  The ones that fall under this category but are still understood by the replicator, are listed here.

###ToggleValve
###toggle_motor1
###toggle_fan
###set_servo1_position
###SetMotor1Speed

##Errors found
All errors listed below were discovered when connecting to a Replicator using version 5.2 of the MB firmware.

###MaximumPayloadSize
The test labeled testMaximumLength is designed to give the Replicator a packet of maximum length and ensure it accepts it.  This test is set up by writing a bytearray of length MaximumPayloadSize - 4 (we use the MaximumPayloadSize less 4 since the packet contains 4 bytes in addition to the payload: header [1], offset [2] and length [1]).  After sending, a transmission error is thrown, indicative of the replicator not accepting the packet.

### init
I set the replicator's position to [10, 9, 8, 7, 6].  I then gave it five delay commands of 1 second each.  After calling init, the buffer was successfully cleared, but the position was set to 10, 9, 8, 7, 6.

###toolhead_init
I set the toolhead temperature to 100C, then call toolhead init.  The ToolheadTargetTemperature is not reset back to 0, and remains at 100C.

### Get Next Filename
I supply a set of testFiles in the s3g repo that the tester should burn onto their SD card.  It contains only one file, named box 1.s3g.  The tester supplies the volumeName of their sd card, and get_next_filename is used to query the replicator for filenames.  According to the spec, this function should return either the volume name or filenames of the sd card.  Also, passing in a non-zero value will reset the pointer to the beginning of the directory.  I pass in True for the first call, which is supposed to give me the first item in the directory.  Intuitively, this probably should be the volume name, since its spec'd that it will return the volume name.  But, instead I get the first file on the SD card.

###get_motherboard_status
This function has yet to be implemented into the firmware (at least for the most recent pull (5/3 at 18:49) of the master branch)

###get_communication_stats
This function is spec'd to use number 26, but the latest pull of the firmware (5/3 at 18:49) has it defined to 25

###build_end_notification
I start a build named 'test' that is specified to be 10 commands long.  I then send the build end notification and check to make sure the current build name is returned as a null terminated string.  However, its still returned as test.

###delay
Should delay in microseconds, delays in miliseconds instead

###queue_extended_point_new
I set the replicator's position to [5, 6, 7, 8, 9].  I then used queue_extended_point_new to move the replicator in an absolute manner to [1, 2, 3, 4, 5].  I waited for the replicator's move to finish, and checked the its position (expecting [1, 2, 3, 4, 5]).  Instead, I got [43, 6, 3, 4, 5].  

###get_tool_status
I first test to make sure the return values are correct for a machine in the ready state (extruder(s) ready, no errors).  I then Set toolhead 0's temperature to 100 and compare get_tool_status' 0th bit (which is set if the Extruder has reached target temperature) to IsToolheadReady (which returns true if the toolhead has reached temperature, false otherwise).  IsToolheadReady returned False (toolhead not ready), and get_tool_status' 0th bit was set (toolhead is ready).

###wait_for_button
First, the user gives feedback regarding whether or not  waitForButton works with no timeout.  Then timeouts are tested.  A timeout of 5 seconds is specified, and the user watches the replicator to make sure it stops waiting for a button push after 5 seconds.  _It does not_.   The replicator then waits for a button push with a 1 second timeout, with the resetAfterTimeout flag set as true.  Instead of resetting once, it goes into a constant loop of resetting.
_tldr_
Throwing the timeout reset flag, and letting the timeout elapse causes the mightyboard to be in a constant loop of resetting.
Throwing the Timeout ready state flag and letting the timeout elapse causes the center button to constantly flash

###set_potentiometer_value
I measured the vRef of both the A and B axes WITHOUT the botstep plugged in.  Using set_potentiometer_value, I set the potentiometer value to 0.  According to the MCP4018 spec sheet, the resistance of the digipot is equal to ((Rab)n)/127+Rw, where Rab is 5k, N is the pot value we specify (which is 0 in this case) and Rw is the resistance of the wiper, which is 100 ohms.  This gives us 100 ohms for the digipot resistance.  The vRef is ascertained by using r2/(r1+r2)(5V), where r1 is the 10k ohm resistor and r2 is the digipot (r1 and r2 are opposite each other relative to the vRef.  See the schematic for more information).  Using this, the vRef should have been .05V.  My reading was about 1.98V.

###set_RGB_LED
The replicator's LED colour is first set to green, and the user is asked to verify.  The LEDs are then set to the same colour (green), with a blink value of 128.  The LEDs do not blink  (values of 0 and 256 were tried as well, with the same results)
The replicator's LED colour is first set to green, and the user is asked to verify.  The LEDs are then set to the same colour (green), with a blink value of 128.  The LEDs do not blink  (values of 0 and 256 were tried as well, with the same results)
Setting different colours works fine, but setting a blink value of either 0, 128 or 256, nothing happened

###set_beep
The replicator is given a frequency of 261.626 hz for 3 seconds.  The user is then asked if there was any audible sound coming from the piezo buzzer.  No sound is heard.
