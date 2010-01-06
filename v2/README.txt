# Preamble: GPLv3, etc.

-- INTRODUCTION

This is the codebase for the RepRap/Makerbot firmware suite.  The
currently supported boards are:
* RepRap Motherboard, v1.2
* Makerbot Motherboard, v2.0

The toolhead boards supported by the extruder toolhead codebase are:
* Extruder Controller v2.3
* Extruder Controller v3.0

This implementation uses the Gen3 v1 packet protocol as described
here:

-- REQUIREMENTS

To build the firmware, you will need to install:
* avr-gcc
* avr-libc
* scons

To build and execute the tests, you will need to install:
* simavr
* googletest

-- BUILD DIRECTIONS

Run scons from this directory, specifying the target board.  By
default a build will be made for the RRMBv1.2.  For example:
# scons platform=rrmb12

You can upload the firmware to a board by running scons with the
upload target:
# scons platform=rrmb12 upload

-- TEST DIRECTIONS

...

