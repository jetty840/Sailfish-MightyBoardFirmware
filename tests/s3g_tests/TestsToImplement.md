#Replicator s3g Tests we can possibly add


### 152 - Reset to Factory
    Incorporate eeprom test from "ReplicatorStateTests.py"

### 36 - toolhead status
    Incorporate tests from ReplicatorStateTests.py


## Tests that need some work and are not a priority

### 14-17 SD card tests
    Come back to the set of SD card tests at a later point
    include tests for BOTBUILDING error codes

### 37 - PID state
    there are no function tests - do we want them?

### 141 - Wait for platform ready
### 135 - Wait for tool ready
    set temp, wait for temp, run a build start command after, poll for build started and tool ready

## Tests with no good way to do a non user required function test

* 27 - Advanced Version number
* 134 - Change Tool
* 137 - Enable/Disable Axis
* 145 - Set Pot Value
* 146 - Set LED
* 147 - Set Beep
* 148 - Wait for Button
* 149 - Display Message
* 150 - Set Build Percentage
* 151 - Queue Song
* 00  - Version Numbers
* 12 - Enable/Disable Fan
* 13 - Enable/Disable Extra
* 23 - Motherboard Status

## Tests requiring external tools

* Get Platform / Tool Temperature

