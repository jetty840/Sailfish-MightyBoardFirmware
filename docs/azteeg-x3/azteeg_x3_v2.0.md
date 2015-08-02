# Sailfish for Azteeg X3 V2.0
29 July 2015

## Introduction
Sailfish is presently in beta test on Panucatt Azteeg X3 V2.0 electronics.  Sailfish
is primarily designed to squeeze the most performance possible out of 8 bit, 16 MHz
AVR microprocessors.  Sailfish grew out of the RepRap Gen 4 electronics and firmware
and borrows Marlin's acceleration planner, itself derived from Sprinter and grbl.
Some important differences from Marlin and other RepRap firmwares are

* Sailfish's LCD UI is a five-button UI: UP, DOWN, LEFT, RIGHT, and CENTER/OK/SELECT.
On the ViKi 1, the rotary encoder is also supported and acts as an up and down
scroll.  Use of the LCD UI is describe in [Section 3.2](http://www.sailfishfirmware.com/doc/ui-main-menu.html#x12-110003.2)of the Sailfish documentation.

* For a slight performance boost, Sailfish still uses the RepRap Sanguino Gen 3
communications protocol (aka, [Sanguino3 Gcode](https://en.wikipedia.org/wiki/Sanguino3_G-Code), S3G). Sailfish does not directly consume gcode; gcode must first be
translated to S3G or extended S3G, X3G.  This may be done with Dr. Henry Thomas'
open source [GPX](https://github.com/dcnewman/GPX/releases), with the somewhat
defunct ReplicatorG 40 - Sailfish, with Mark Walker's GPX-based plugin for
Octoprint, or with either of the proprietary slicers MakerBot MakerWare/Desktop
or Simplify 3D's Simplify 3D slicer (which incorporates GPX).  Gcode files placed
on SD cards will be ignored by Sailfish: only directory folders and files with
the (case insenstive) extensions `.s3g` or `.x3g` will be honored.

* For the most part, there are no gcode commands to set printer parameters. Briefly,
Sailfish supported some but they were removed to save code space in the cramped
ATmega 1280s which MakerBot was using.  Printer parameters are set through a
combination of the LCD UI and ReplicatorG 40 - Sailfish.  Not everything can
presently be set through the LCD UI: another side effect of catering to the
ATmega 1280.  And not everything can be set via ReplicatorG, but that's usually
just because ReplicatorG lags behind changes in Sailfish.  There are plans to
update the Sailfish LCD UI on ATmega 2560 systems to provide access to all
firmware parameters (such as is possible on Thing-o-Matics).

* The [acceleration parameters in Sailfish](http://www.sailfishfirmware.com/doc/parameters-acceleration.html#x21-590004.3)
should be familiar to Marlin users. In place of the "jerk" parameters, Sailfish
uses limits on maximum speed changes along each axis.  Missing are Marlin/grbl's
maximum normal and extrusion parameters. If you carefully look at Marlin and grbl,
you will see that those parameters have no effect: they are overridden by per-axis
maximum accelerations.  Thus they serve no purpose.  Well, that's almost true: should
the maximum normal or extrusion acceleration be set to a value smaller than one of
the per-axis maximum accelerations, then it will prevail over the per-axis maximum.
But the same effect can be had by simply reducing the per-axis maximum and again
obviating the need for the maximum normal or maximum extrusion acceleration.
Consequently, Sailfish does without the maximum normal and extrusion accelerations,
sticking to just the per-axis maximums.


## Documentation
Fairly extensive documentation for Sailfish may be found at the [Sailfish documentation
site](http://www.sailfishfirmware.com/doc/index.html).  As of this writing that
documentation has not yet been updated with information on Azteeg X3 support.


## Wiring
Wiring an Azteeg X3 V2.0 board for Sailfish is, for the most part, straightforward.
In the description that follows, the terminal/connection references are as per the
wiring diagrams below and available as the files [x3_v2_wiring-1.png](https://github.com/jetty840/Sailfish-MightyBoardFirmware/blob/master/docs/azteeg-x3/x3_v2_wiring-1.png)
and [x3_v2_wiring-2.png](https://github.com/jetty840/Sailfish-MightyBoardFirmware/blob/master/docs/azteeg-x3/x3_v2_wiring-2.png).

![x3_v2_wiring-1](./x3_v2_wiring-1.png "Main board wiring")
![x3_v2_wiring-2](./x3_v2_wiring-2.png "Cover board wiring")

* Tool 0 (right extruder)
	* Heater: wire to the main board's first hot end terminals, H-END.
	These are the two terminals closest to a board corner.
	* Thermistor: wire to the T0 thermistor terminals if using a thermistor.
	* Thermocouple: wire to the expansion board's TC1 terminal if using a
	thermocouple.  Negative is the outermost (leftmost) terminal.  From the LCD
	UI, configure Tool 0 to use a thermocouple.
	* Heatsink cooling fan: if using an extruder heatsink cooling fan, wire to
	the cover board's H-END 3 terminals.  Positive is the outermost terminal.
	Turns on when the extruder temperature is at or above 50C.  (Configurable
	via EEPROM with ReplicatorG.)

* Tool 1 (left extruder; optional)
 	* Heater: wire to the main board's second hot-end terminals, H-END.
 	These are the two terminals closest next to the heated bed terminals, H-BED.
	* Thermistor: wire to the T1 thermistor terminals if using a thermistor.
	* Thermocouple: wire to the expansion board's TC2 terminal if using a
	thermocouple.  Postive is the outermost (rightmost) terminal.
	* Heatsink cooling fan: if using an extruder heatsink cooling fan, wire to
	the expansion board's H-END 4 terminals. Negative is the outermost terminal.
	Turns on when the extruder temperature is at or above 50C.  (Configurable
	via EEPROM with ReplicatorG.)
	* When adding a second extruder, from the LCD UI or ReplicatorG, you can
	set the extruder count to 2.  Power cycle the printer after changing
	this setting.  You will then want to calibrate the X and Y toolhead offsets.

* Heated bed (optional)
	* Connect heater PCB to the heated bed terminals, H-BED.
	* Thermistor: connect to the TBED thermistor terminals.
	* If you do not use a heated bed, then from the LCD UI or RepG, you can
	disable the use of a heated bed.  Power cycle the printer after changing
	this setting.

* Endstops
	* Z minimum and Z maximum are as per the Azteeg silkscreen.
	* Since many MakerBot style printers use X and Y maximum homing but
	the Azteeg only provides terminals for minimum endstops, Sailfish
	switches the min and max endstops for X and Y.  To hook up X and Y max
	endstops, connect to the X-min and Y-min terminals on the Azteeg.
	* By default the endstops are assumed HIGH when inactive and LOW when
	triggered.  This can be changed via RepG.
	
* Print cooling fan (optional)
	* Connect to D4 column of "LOW POWER SWITCH (PWM)" terminals.  Note
	that X3G does not accomodate PWM fan speed control.  A print cooling
	fan is either full on or full off.  However, Sailfish does have an
	EEPROM setting (accessible from the LCD UI) to set a 0 - 100% duty
	cycle to use when the print cooling fan is enabled.
	
* RGB LEDs (optional)
	* Sailfish presently builds for the Azteeg assuming common anode LED
	strips.  Wire the +12/+24V common wire to the +12V terminal at D5,
	D6, or D11 on the "LOWER POWER SWITCH (PWM)".  Then wire the red strand
	to the SW position for D5; green to the SW position for D6; blue to the
	SW position for D11.
	
* ViKi 1 LCD UI (optional, but strongly recommended)
	* Wire as per Panucatt's [viki_wiring_diagram.pdf](http://files.panucatt.com/datasheets/viki_wiring_diagram.pdf).
	* Follow the Azteeg X3 wiring; the "button pin" is presently not supported.
	Ideally, it would be used for the Pause-Stop function but future support
	for the ViKi 2 may use it for a LEFT button (exit menu).
	
	![Wiring the ViKi 1; ViKi 1 board](./viki_wiring_diagram-1.pdf "ViKi 1 wiring")
	![Wiring the ViKi 1; Azteeg X3 board](./viki_wiring_diagram-2.pdf "ViKi to Azteeg wiring"")


## Installing Sailfish
To install Sailfish, refer to the [Installing Sailfish documentation](http://www.sailfishfirmware.com/doc/install-installing.html).  However, since the Azteeg port
Sailfish is still in beta test, you must provide ReplicatorG with a different
download URL,

1. With ReplicatorG running, but not connected to your printer, go to the
Preferences submenu which appears under the File menu on Windows and Linux
and under the ReplicatorG menu on OS X.
2. In the Preferences window, click on tha Advanced tab.  Set the "Firmware
update URL" to `http://jettyfirmware.yolasite.com/resources/beta/firmware.xml`
and then close the Preferences window.
3. You may need to exit and restart ReplicatorG to get it to then download the
beta firmwares.  You should see it logging the downloads in logging section
at the bottom of the main window.  (Note that you must be using revision 30 (r30)
or later of ReplicatorG 40 - Sailfish for downloads to work owing to changes
in the CloudFlare web server caching service used by yola.com.)
4. Note that you do not need to press any reset button on the Azteeg when
downloading firmware to it.
5. After installing Sailfish, it is highly recommended that you do a full reset
on the EEPROM.  Sailfish will check the first couple of bytes of the EEPROM
for version information and do a full reset if what it finds does not match
expectations.  However, it might be best to be certain.  From the LCD UI this
can be done from the EEPROM section of the Utilities menu using the "Erase EEPROM"
selection.  See the [Section 3.7.18](http://www.sailfishfirmware.com/doc/ui-utilities-menu.html#x17-540003.7.18)
of the Sailfish documentation for details.  Alternatively, ReplicatorG 40 - Sailfish
may be used.  This is done with the "Reset motherboard entirely" button of the
"Onboard Preferences" window accessed via the Machine menu.  Regardless of the
means you use to reset the EEPROM, do so before comissioning the printer;
otherwise, EEPROM settings effected whilst commissioning will be erased and
reset to their defaults.

If you prefer to use `avrdude` directly, you may do so.  However, you will
need to use ReplicatorG 40 - Sailfish when commissioning your printer. As of
this writing, the current beta of Sailfish for the Azteeg is r1600 and the
`.hex` files may be downloaded from

* Cartesian Core-XY: [azteeg-x3-corexy-Sailfish-v7.8.0-r1600.hex](http://jettyfirmware.yolasite.com/resources/beta/firmware/azteeg-x3-corexy-Sailfish-v7.8.0-r1600.hex)
* Standard Cartesian: [azteeg-x3-Sailfish-v7.8.0-r1600.hex](http://jettyfirmware.yolasite.com/resources/beta/firmware/azteeg-x3-Sailfish-v7.8.0-r1600.hex)

If using `avrdude` via the USB port of the Azteeg, then the pertinent
`avrdude` switches are `-p m2560`, `-c stk500v2`, and `-b 115200`.


## Commissioning

### Axis lengths, maximum axial speeds, steps/mm values
To commission a printer with Sailfish, you need ReplicatorG 40 - Sailfish
("RepG") to upload the lengths of each axis, maximum axial speeds, and axial
steps/mm values.  It is important that you use RelicatorG 40 - Sailfish and
not the old ReplicatorG&nbsp;0040 which MBI abandoned in November 2012.  Before
abandoning it, MakerBot accidentally introduced some bugs in it which, for
any firmware reporting a version of 7.0 or later, will corrupt some EEPROM
settings.

ReplicatorG 40 - Sailfish may be downloaded from the
[Sailfish Thing](http://www.thingiverse.com/thing:32084/#files) at
[thingiverse.com](http://thingiverse.com).

Before running RepG, edit a copy of [this sample machine definition file](https://raw.githubusercontent.com/jetty840/Sailfish-MightyBoardFirmware/master/docs/azteeg-x3/azteeg.xml)
and save the result in the directory `~/.replicatorg/machines/azteeg.xml`.  (If
you have never run RepG before, then you will need to first create those directories.)

	<machine>	  <name>My Azteeg X3</name>	  <geometry type="cartesian">	    <axis id="x" length="200" maxfeedrate="18000" homingfeedrate="2500" stepspermm="88.89" endstops="max"/>	    <axis id="y" length="200" maxfeedrate="18000" homingfeedrate="2500" stepspermm="88.89" endstops="max"/>	    <axis id="z" length="210" maxfeedrate="600" homingfeedrate="600" stepspermm="400" endstops="min"/>	    <axis id="a" length="100000" maxfeedrate="1600" stepspermm="96.28" endstops="none"/>	    <axis id="b" length="100000" maxfeedrate="1600" stepspermm="96.28" endstops="none"/>	  </geometry>	  <driver name="mightysailfish">	    <rate>115200</rate>	  </driver>	  <tools>	    <tool name="extruder" model="Mk8" diameter="0.4" stepper_axis="a" index="0" type="extruder"		  motor="true" fan="true" heatedplatform="true" motor_steps="3200" default_rpm="3" heater="true"/>	    <tool name="extruder" model="Mk8" diameter="0.4" stepper_axis="b" index="1" type="extruder"		  motor="true" fan="true" heatedplatform="true" motor_steps="3200" default_rpm="3" heater="true"/>	  </tools>	  <bookend dualstart="machines/replicator/Dualstrusion_start.gcode"		   start="machines/replicator/Dual_Head_start.gcode"		   end="machines/replicator/Dual_Head_end.gcode"/>	</machine>
In the file, only edit the `<axis>` elements in the `<geometry>`
section.  Do not edit the other sections: the information in them must be
present but will not be sent to Sailfish.  __Leave them alone__: mistakes in
those sections can lead to mysterious RepG hangs.

In the `<axis>` elements, the only fields you need to change are

1. `length="..."` specifies the length of the axis in units of millimeters.
 The firmware only cares about the length of the Z axis.  It uses that
 value when clearing the platform during a print pause.
2. `maxfeedrate="..."` specifies the maximum feedrate for that axis in units of
 millimeters per minute.  This is used by the firmware to limit top speeds when
 the printing speed is dynamically changed during a print.  When converting gcode
 to X3G, the converter itself will impose maximum feedrates.  As such, the
 firmware only needs to know the maximum feedrates for when it is told to increase
 them past those in the X3G commands.
3. `stepspermm="..."` is the number of microsteps required to move the axis one
 millimeter.  Knock yourself out with high precision values.  They will be stored
 as single precision floating point values in EEPROM.  The firmware uses these
 values when clearing the build platform during a pause -- it will move within
 5mm of each endstop.  The values are also used to convert between units of
 steps per second and millimeters per second.  I.e., used to impose kinematical
 constraints such as maximum rates of acceleration.

Do provide information for two extruders, axes A and B, even if you will only
be using a single extruder.

Once you have edited the XML file and saved it to `~/.replicatorg/machines/azteeg.xml`,
launch ReplicatorG.  When ReplicatorG is running, go to the "Machine Type (Driver)"
submenu of the Machine menu.  Select the machine "My Azteeg X3".  If you do not
see that machine listed, then you likely mis-edited the XML file causing it to
be syntactically invalid XML.  Correct the mistake and try again.

Once "My Azteeg X3" is selected, connect to your printer over USB:

1. Connect the USB cable between your printer and your computer.
2. Power the printer on.
3. In ReplicatorG, use the "Connection (Serial Port)" submenu of the Machine
 menu to select the comms port for your printer.  If RepG was running before
 you powered your printer on, then use "Rescan serial ports" choice of the
 submenu.  Then, once the ports have been scanned, select the port.
4. Now, click the "Connect" icon in RepG: it's the second icon from the
 right.
5. Once connected, RepG will query the printer's and automatically upload
 any machine settings from the XML file which differ with those in the
 printer's EEPROM.
6. You can also use the "Onboard Preference" submenu of the Machine menu
 to see and change settings in the EEPROM.  For example, the maximum
 acceleration and speed change settings.
 
See [Section 6.4.2.2 of the Sailfish documentation](http://http://www.sailfishfirmware.com/doc/install-configuring.html#x35-900006.4)
for an example of using RepG to connect to a printer.  [Chapter 4](http://www.sailfishfirmware.com/doc/parameters.html#x18-560004)
of that documentation describes all the firmware parameters available.


### Tool count and heated bed
Sailfish defaults to assuming you have one extruder, Tool 0, and a heated
bed.  To change these, you can either use ReplicatorG or the LCD UI:

* From the LCD UI, select the Utilties menu.  From Utilites, select "General
Settings".   **See [Section 3.7.4](http://www.sailfishfirmware.com/doc/ui-utilities-menu.html#x17-280003.7.4)
for instructions on how to use this menu.**  Set the "Extruder Count"" to 1
or 2 and set the "HBP Installed" to YES or NO.  After changing either or both
of these, power cycle your printer. See Sections 3.7.4.6 and 3.7.4.8 of the
Sailfish documentation for further information. 


### Thermistor types
Presently, Sailfish on Azteegs defaults to assuming that Epcos 100K thermistors
(Marlin table 6) are used for all temperature sensors (bed, tool 0, and tool 1).
You must use the LCD UI to select different thermistor tables (per sensor) or to
use a Type-K thermocouple for either or both of the extruders.  RepG has not yet
been updated to make these changes to your printer.

From the LCD UI,

1. Select the Utilties menu from the top-level menu.
2. With the DOWN button, scroll down to the "Temp Sensor Types" item.  It will
be down a screen or two.
3. Select "Temp Sensor Types" with the CENTER button.  (Often referred to as
the "M" button dating back to MakerBot's Replicator 1 which had an "M" on that
button.)
4. Scroll down or up with the DOWN and UP buttons to see the different thermistor
choices.  The numeric values shown as part of the terse description/name is the
Marlin thermistor table number.  Presently tables 1 through 7 are available.  To
use a thermocouple with either of the extruders, use the "0. K Thermocouple" choice.
After scrolling to the desired choice, press the CENTER button.  As you make
your choices, you will progress from Tool 0 (right/primary extruder) to Tool 1
(left extruder) to the heated bed.  Once you press the CENTER button for the bed
thermistor, the settings will be effected and you will be returned to the
Utilities menu.  If you press the LEFT button at any time, you will exit the
"Temp Sensor Types" menu and your choices will be discarded.

Note that the "Temp Sensor Types" menu will always set choices for all three
temperature sensors regardless of whether you have enabled or disabled Tool 1
or the heated bed.

	
## Configuring GPX