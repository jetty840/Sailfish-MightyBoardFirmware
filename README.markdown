	Table of Contents
	1.0 Introduction
		1.1 Advanced printing features
		1.2 SD card features
		1.3 Robustness features
		1.4 General features
	2.0 Before Building
	3.0 Building Sailfish
		3.1 Advanced building
	4.0 Credits

**1.0 Introduction**

The Sailfish firmware is an enhanced firmware for Makerbot printers containing
features intended for advanced users.  These features fall into several broad
categories:

  1. Advanced printing features such as changing the printing speed and
     temperature while printing.
  2. More robust error handling.
  3. Additional features such as support for high capacity SD cards and file
     folders on SD cards.
  4. Support for different bot mechanics and drive systems (e.g., Core-XY,
     H-gantry, different sized axes, different motion constraints, different
     LCD hardware, alternate serial comms, etc.).

The bot's firmware is the software which resides within the bot and controls
the bot's behavior.  It is the software which receives printing instructions
from MakerWare, ReplicatorG, SD card files, and other desktop programs and then
executes them to create your 3D print.  Sailfish has existed since late 2011 and
is based upon a combination of the RepRap Marlin and MBI Gen 4 firmwares.  Sailfish
builds upon Marlin with an improved "Advance" algorithm as well as a five to six
times faster acceleration planner.  (MBI independently measured it to be ten times
faster relative to their own port of Marlin.)  The faster planner is achieved by
using fixed point arithmetic rather than floating point math which the microprocessor
in Makerbots and many DIY 3D printers cannot do in hardware and must emulate in
software.  Additional performance gains are had through careful optimization of
the computations (e.g., elimination of frequent square and square root calculations
by working in velocity-squared space rather than velocity space).

Sailfish was originally known as the "Jetty Firmware".  In early Fall of 2012, MBI
adopted the core acceleration and printing control code of Sailfish into their own
firmware, abandoning their own port of Marlin.  However, MBI's firmware does not
include many of the more advanced features of Sailfish -- in part because of the
limited program space in Makerbots and MBI's need to use much of that space for
"first run" features.

What follows is a description of features found in Sailfish which are not part of
the MBI firmware for Makerbots.  A good number of these features are also found
in RepRap firmwares; they are not necessarily unique to Sailfish.

**1.1 Advanced printing features**

  1. Change the printing speed from the LCD display while printing.
  2. Change the extrusion temperature from the LCD display while printing.
  3. At the end of a print, the total print time and the amount of filament
     used is displayed.
  4. At the end of a print, you can elect to repeat the same print: no need
     to go through screens reselecting the file. Quite useful when asking
     an associate, spouse, or child to repeat a print several times.
  5. "Ditto" printing for dual extruder printers: simultaneously print the
     same model twice using both extruders or do other clever tricks.
  6. Override temperatures found in gcode with different temperatures: useful
     for redoing a series of prints at different temperatures without having
     to reslice.
  7. Control whether or not heaters are left active when prints are paused.
  8. Jog the axes when a print is paused so as to make mid print filament
     loading easier. The axes' position is automatically restored when the
     print is resumed.
  9. On printers with a most two heaters, a line of the LCD display provides
     constantly updated build statistics: filament used, percent complete,
     elapsed time, estimated time remaining, and current height.
 10. Much finer control of the height to pause at with Pause @ ZPos: MBI's
     firmware only allows specification to the nearest millimeter whereas
     Sailfish allows 0.01 mm resolution.
 11. Multiple Pause @ ZPos positions may be specified in gcode.
 12. When used in conjunction with GPX, automatic compensation for different
     filament diameters when placing Pause @ ZPos commands within your gcode.
 13. Support for Emergency stop and Pause stop hardware.  Pause stop hardware
     allows a print to gracefully be paused when hardware detects a temporary
     problem such as filament running out or being snagged.  Once the condition
     is resolved, the print may be resumed.

**1.2 SD card features**

  1. Support for file folders on SD cards.
  2. Support for high capacity SD cards (e.g., 4, 8, 16, 32 GB, etc.).
  3. Support for the FAT-32 file system.
  4. Support for the Toshiba AirFlash Wi-Fi card: send files to your bot
     over Wi-Fi.
  5. SD card error detection and correction.
  6. Long filenames are allowed; long file names scroll in the display so that
     the entire name can be viewed. MBI's firmware limits file names to 15
     characters, not including the mandatory, case-sensitive ".x3g" extension.
  7. Ability to save and restore the bot's internal settings -- onboard
     preferences -- to an SD card.  Backup your bot's settings or quickly
     transfer them to another bot.

**1.3 Robustness features**

  1. Improved error handling and reporting of heater errors. For example, on
     bots with more than one heater, the heater errors indicate which heater
     the error message refers to.  No more trying to figure out if it's your
     HBP or extruder (and which one) which is failing.
  2. MBI's firmware does not return errors when printing over USB.  This
     typically results in gcode instructions being dropped. Often that is
     benign and merely results in a slight printing blemish (i.e., a dropped
     G1 command), but in some instances can and will result in prints being
     ruined. For example, there's been cases of commands to re-enable stepper
     motor currents being lost, and commands to turn off extruders lost.
     Sailfish actually returns errors when printing over USB so that MakerWare
     and RepG can intelligently handle the problem.
  3. Improved SD error messages: finer detail on what failed with the SD card
     (http://jettyfirmware.yolasite.com/v73-v43.php#sderror).  MBI's firmware
     often produces a message indicating that the SD card is not formatted as
     FAT-16 when the actual error is something else entirely. This because the
     firmware lumps about five or six different error cases into one.
  4. Additional safety features:
     http://jettyfirmware.yolasite.com/v74-v44.php#safety.

**1.4 General features**

  1. Support for Core-XY and H-gantry drive systems and other alternate hardwares
     (e.g., I2C LCDs, alternate serials comms, alternate thermocouple chips, etc.).
  2. Track your filament usage: storage and recall of used filament counters;
     filament "odometers".
  3. Better print quality. Sailfish is careful to run critically timed
     operations at the highest priority in the microprocessor.  Unfortunately,
     not so in the MBI firmware. When printing very fine detail at high print
     speeds, this can make a difference. Also, when printing over USB, the fact
     that the firmware returns error messages actually leads to improved print
     quality. When no error message is returned as is the case with MBI's
     firmware, MakerWare and RepG actually pause waiting for a response and
     then just resend the same command which may then fails again, leading
     to another pause. These pauses can leave the bot sitting idle, producing
     a small, extra blob of plastic on the print and visible as pimples when
     they occur on the outer surface. By returning error messages, these
     pauses are avoided.
  4. Improved levelling script: allows you to move the extruder to any
     location over the build platform as many times as you want and however you
     want. No more checking just a few predefined points.
  5. Extruder hold feature for use with 3mm extruders which have higher
     internal pressure and thus more significant problems when the gcode
     gratuitiously disables the extruder motors. This feature allows gcode
     commands to disable the extruder to be ignored. (Some slicers like to
     generate many such commands.)
  6. More precise and stable heater control: MBI's firmware introduces an
     additional +/- 1C of measurement error in their PID control.

**2.0 Before Building**

In order to build Sailfish, you must first install

  1. An avr-gcc toolchain.  Version 4.6.2 is strongly recommended.  Earlier
     versions have a bug in the floating point implementation.  Later versions
     will present difficulties owing to vacillation in the methods of declaring
     PROGMEM data.  If you use a different versionof the avr-gcc toolchain,
     then you're on your own.
	
  2. The SCons build tool.  MBI chose to use SCons for their build tool and
     Sailfish follows that choice.  You will need to install SCons ("scons").
	
See the markdown document `docs/avr-gcc.markdown` contained in the Sailfish
source distribution for information on building an avr-gcc toolchain.  Additionally,
there are scripts in `dist/build-avr-gcc/` to build an avr-gcc toolchain.  The
script for cygwin is a "work in progress".

**3.0 Building Sailfish**

To build Sailfish move to the `firmware/` directory of the source distribution
and issue a scons command of the form shown here,

	% cd firmware
	% scons platform=<platform-name>
	
where `<platform-name>` is the name of a supported platform (e.g., `mighty_one`).
To obtain a list of all platform names, issue the command

	% python src/platforms.py

As of this writing, the output of that command is

	% python src/platforms.py
	mighty_one-corexy ff_creator-2560 ff_creatorx-2560 wanhao_dup4 mighty_twox
	ff_creator mighty_one mighty_one-2560-corexy mighty_twox-2560 mighty_two
	mighty_one-2560 mighty_two-2560 mighty_one-2560-max31855

Note that `mighty-one` is the name that Makerbot gave to the firmware for
the original MightyBoard -- the MightyBoard rev E -- used in the Replicator 1.
Sailfish uses the same name as well as the name Makerbot gave for the Replicator 2
firmware,

	% scons platform=mighty_two
	
However, Makerbot uses the same platform name -- `mighty_two` -- for both the
Replicator 2 and Replicator 2X.  Sailfish takes a departure and instead builds
separate firmwares for the two.  To build firmware for the Replicator 2X, use
the platform name `mighty_twox`,

	% scons platform=mighty_twox

By doing this, Sailfish saves code space in the Replicator 2 build by not
including features needed by the Replicator 2X.  That saved code space is then
used to provide additional features for the Replicator 2 such as the running
build stats ticker showing elapsed time, estimated time remaining, filament
used, etc.

After building a binary, the resulting hex file may be found in the directory

	firmware/build/<platform-name>/

**3.1 Advanced building**

When building, additional features may be compiled in by means of command
line parameters to the scons command.  See the `src/SConscript.mightyboard` file
for all the possible parameters.  For example, to generate a Replicator 1
build for a board using the MAX31855 chip, issue the command

	% scons platform=mighty_one max31855=1

Additional parameters may be supplied on the command line.  E.g.,

	% scons platform=mighty_one max31855=1 core_xy=1

General compiler defines, -Dxxxx, can also be supplied.  The command,

	% scons platform=mighty_one defines=AUTO_LEVEL,DEBUG

will add `-DAUTO_LEVEL` and `-DDEBUG` switches to each compile command.

If you will often be building a custom build, consider writing your own
platform definition and placing it in the file

	~/.sailfish_platforms.py

See the file `firmware/src/platforms.py` for information on the contents of
that file.  The following sample file defines a platform named `franken-board`
which is, essentially a Replicator 1 with an ATmega2560, Core-XY drive train
(CORE_XY), additional serial comms support (ALTERNATE_UART), MAX31855 chips
(MAX31855), a large power supply which can drive all heaters and steppers
concurrently (HEATERS_ON_STEROIDS), and includes support for the running
build stats ticker display on the LCD (BUILD_STATS),

	% cat ~/.sailfish_platforms.py
	platforms['franken-board'] = {
    	'mcu' : 'atmega2560',
    	'programmer' : 'stk500v2',
    	'board_directory' : 'mighty_one',
    	'defines' : [ 'CORE_XY', 'BUILD_STATS', 'ALTERNATE_UART',
    				   'HEATERS_ON_STEROIDS', 'MAX31855' ]
	}

This platform is then built with the command,

	% scons platform=franken-board

Multiple platforms can be defined in the file.  E.g., the following defines
a second board, `another-board`, which is a Replicator 2 style bot (`mighty_two`)
with many of the same characteristics as the `franken-board` platform,

	% cat ~/.sailfish_platforms.py
	platforms['franken-board'] = {
    	'mcu' : 'atmega2560',
    	'programmer' : 'stk500v2',
    	'board_directory' : 'mighty_one',
    	'defines' : [ 'CORE_XY', 'BUILD_STATS', 'ALTERNATE_UART',
    				   'HEATERS_ON_STEROIDS', 'MAX31855' ]
	}
	platforms['another-board'] = {
		'mcu' : 'atmega1280',
		'programmer' : 'stk500v1',
		'board_directory' : 'mighty_two',
		'defines' : [ 'CORE_XY', 'BUILD_STATS', 'ALTERNATE_UART',
    				   'HEATERS_ON_STEROIDS' ]
    }

**4.0 Credits**

This software incorporates code related to acceleration from Marlin:

	https://github.com/ErikZalm/Marlin

This software is covered by GNU General Public License v3 and according to
Section(7), Subsection (b), additional permissions for author  attribution
are required on any work that incorporates, is derived or inspired from the
following components:

  1. JKN Advance
  2. YAJ (Yet Another Jerk)
  3. Altshell ReplicatorG plugin
  4. Pause @ ZPos
  5. Advance Pressure Relax

Author attribution is required as follows:

  1. If the device this software is executed on has an LCD or
     display screen attached, credit/attribution must be provided
     on this screen with at least 1 second duration when and each
     time the device is powered on.
 
  2. If the device does not have a display screen attached and the
     software is distributed in binary form, then credit/attribution
     is required to be displayed to the user prior to installation of
     the binary software.
 
  3. If the software is being supplied in source code form, then any
     existing credit/attributions must be retained.

  4. If the software is being supplied in source code form, but it is
     derived or inspired from this source code, then Credit/Attribution
     below must be provided in the source code near the top of the source
     file.

  5. The additional permissions listed here are required to be included
     in their entirety with any license file of any derivative works that
     use the above features.

Credit/Attribution:

	This software uses the following components from Jetty Firmware:
	(LIST OF COMPONENTS GOES HERE)
	authored by Dan Newman and Jetty.
