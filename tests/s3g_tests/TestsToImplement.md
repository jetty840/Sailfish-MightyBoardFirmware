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
    No tests exist

### 139 - Queue Extended Point
    The function test doesn't home before running movement commands

### 142 - Queue Extended Point new style
    The function test doesn't home before running movement commands

### 135 - Wait for tool ready
    Write a function test for this

### 141 - Wait for platform ready
    Write a function test for this 

### 145 - Set Pot Value
    We don't currently have the ability to test this, but maybe we will in the future

Commands to Add
GetAdvancedVersionNumber
GetPrintStats
BOTBUILDING, BOT_OVERHEAT, error codes

need to fix tests for setPotentiometer
fix get message command
