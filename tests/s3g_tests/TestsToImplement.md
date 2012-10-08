#Replicator s3g Tests we can possibly add

### 02 - GetAvailableBufferSize
    add test to send commands then test that buffer size has been updated appropriately

### 08 - Pause / resume 
    Pause test - query Pause State

### 13 - Write to EEPROM
    Doesn't have a reply test.  (there is a function test though)

### 14-17 SD card capture
    We don't define for the tests whether an sd card is present

### we don't ensure the z stage is down for motion tests
### build end notification update test to not check for file name - what behavior do we want for returning last print name etc?


### 23 - Motherboard Status
    Doesn't have a function test (does have a reply test)

### 24 - Print stats
    Doesn't have a function test or a reply test (needs to be implemented in s3g)
    Line number test - send lines, check line number (Start build required)
    test pause state
    test canceled - send a reset signal (implement cancel on reset)
    run print for period of time and test print time

### 27 - Advanced Version number
    Does't have a function test or a reply test (needs to be implemented in s3g)

### 133 - Delay
    Implement a function test for delay

### 134 - Change Tool
    No Tests for change tool

### 137 - Enable/Disable Axis
    user tests exist but no function tests

### 139 - Queue Extended Point
    The function test doesn't home before running movement commands

### 142 - Queue Extended Point new style
    The function test doesn't home before running movement commands

### 135 - Wait for tool ready
    Write a function test for this

### 141 - Wait for platform ready
    Write a function test for this 

### 145 - Set Pot Value
    We don't currently have the ability to test this without user interaction

### 146 - Set LED
    We only have a user interaction test - we could test that "custom" eeprom setting is being set correctly

### 147 - Set Beep
    We don't have a way to test this without user interaction

### 148 - Wait for Button
    We don't have a way to test this without user interaction

### 149 - Display Message
    There is only a user interaction test

### 150 - Set Build Percentage
    There is only a user test

### 151 - Queue Song
    There is only a user test

### 152 - Reset to Factory
    Incorporate eeprom test from "ReplicatorStateTests.py"

### 00  - Version Numbers
    Are there non user tests we can / should run for version numbers

### 30-33, 02 - Get/Set Platform/Toolhead temperature
    There should be non-user tests for these

### 36 - toolhead status
    Incorporate tests from ReplicatorStateTests.py


## Tests that need some work and are not a priority

### 14-17 SD card tests
    Come back to the set of SD card tests at a later point
    include tests for BOTBUILDING error codes

### 37 - PID state
    there are no function tests - do we want them?

### 12 - Enable/Disable Fan
    user only test

### 13 - Enable/Disable Extra
    currently no way to function test

### 141 - Wait for platform ready
### 135 - Wait for tool ready
    set temp, wait for temp, run a build start command after, poll for build started and tool ready

Commands to Add
GetAdvancedVersionNumber
GetPrintStats
BOTBUILDING, BOT_OVERHEAT, error codes

need to fix tests for setPotentiometer

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
