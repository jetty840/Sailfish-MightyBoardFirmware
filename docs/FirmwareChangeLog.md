# Firmware Changes Log

## Firmware 5.6 changes
* New menu added for changing settings while printing. 
* Print time now recorded while printing.  Queryable by PC host.  Also visible on the onboard UI
* Current Line Number now visible during printing.  Line number refers to s3g command number, not gcode line number
* Total print time for the bot is stored in eeprom.  This is queryable by s3g host and is visible on the onboard UI 
* SD card menu made more robust to errors and issues cause by static discharge


## Firmware 5.5 changes

* M6 wait issue : bot handles heating the platform.  This fixes the intermittent issue we've seen where the bot stalls after the platform heats up.
* Axis Home Offsets changed : Single Extruder offsets are now: X: 152, Y: 72.  Dual extruder offsets are now: X: 152, Y: 75.   The offset refers to the distance from the center of the right nozzle.  Previous firmware used the middle point between the two nozzles as the reference point for the Dual Extruder machines.  
* Heating error sometimes causes prints to fail during long prints - changed fail counter so that it cannot overflow if there are a few bad reads over a long period of time
* Changed plate leveling script to front-back-left-right-center, changed text to say "tighten four or five times" instead of "tighten all the way"
* Nozzle calibrate script lowers the platform and heats platform first to speed heating when running the script
* Axis inversion defaults to XYZB inverted
* spacing errors fixed in some onboard bot screens
* acceleration added to bot movement
* Cooling Fan on Extruders has hysteresis to prevent turning on and off repeatedly at threshold temperature.
* z clipping added.  if move describes a point longer than the z axes, the bot will only go to max z length (z length can be changed in eeprom settings)

