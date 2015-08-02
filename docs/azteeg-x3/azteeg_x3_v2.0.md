# Sailfish for Azteeg X3 V2.0
29 July 2015

## Introduction
Sailfish is presently in beta test on Panucatt Azteeg X3 V2.0 electronics.  Sailfish
is primarily designed to squeeze the most performance possible out of 8 bit, 16 MHz
AVR microprocessors.  Sailfish grew out of the RepRap Gen 4 electronics and firmware
and, for a slight performance boost, still uses the RepRap Sanguino Gen 3 communications
protocol (aka, S3G).  It does not consume gcode; gcode must first be translated to S3G
or extended S3G, X3G.  This may be done with Dr. Henry Thomas' open source
[GPX](https://github.com/dcnewman/GPX/releases), with the somewhat defunct
ReplicatorG 40 - Sailfish, with Mark Walker's GPX-based plugin for Octoprint, or with
either of the proprietary MakerBot MakerWare or Simplify 3D's Simplify 3D slicer
(which incorporates GPX).

Moreover, with S3G/X3G you do not use gcode to commission a new printer and
set firmware parameters.  Instead, it's done partially through the LCD UI,
partially using ReplicatorG 40 - Sailfish over USB.  (MakerBot MakerWare can also
be used.)  Some of this grew out of the restricted code space of the ATmega 1280
which MakerBot chose to use on several generations of their printers.  As the Azteeg X3
uses an ATmega 2560 with twice the code space, the future may see additional
LCD UI menus to set all of these parameters.

## Documentation
Fairly extensive documentation for Sailfish may be found at the [Sailfish documentation
site](http://www.sailfishfirmware.com/doc/index.html).  As of this writing that
documentation has not yet been updated with information on Azteeg X3 support.

## Wiring
Wiring an Azteeg X3 V2.0 board for Sailfish is, for the most part, straightforward.
In the description that follows, the terminal/connection references are as per the
wiring diagrams below and available as the PNG files [x3_v2_wiring-1.png](http://)

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

## Commissioning
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

Before running RepG, edit a copy of this sample machine definition file and
save the result in the directory `~/.replicatorg/machines/azteeg.xml`.  (If
you have never run RepG before, then you will need to first create those
directories.)

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

	
## Configuring GPX