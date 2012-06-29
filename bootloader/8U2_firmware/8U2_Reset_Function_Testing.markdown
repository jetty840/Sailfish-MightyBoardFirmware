# 8U2 firmware modifications 
June 29, 2012

## Rational For Implementing a Custom Reset
We are using firmware for the 8U2 chip that was developed by the Arduino team. 
This firmware translates virtual Serial DTR line toggles to a RESET output that connects to the reset pin on the Atmega1280.
Serial implementations on Mac OSx and Linux pull the DTR line down on connect and release it on disconnect.  
Mirroring the DTR to the 1280 reset means that the Bot will reset when it is connected to ReplicatorG.
This is undesireable behavior if the Bot is in the middle of a print.

## Fix Implemented
The new firmware ignores virtual DTR line changes.
It pulls the RESET line down when it recieves a new connection with a baud rate of 56700.
57600 is the baud rate we use for firmware updates.
The RESET line is set high for all other baud rates.  Normal communication with the bot uses 115200 baud.

## Test Scenarios
Tests were performed on the following configurations:

Mac OSX 10.7.

Ubuntu 11.10 64bit

Windows 7 64bit


<table>
<tr>
<th>Behavior</th> <th> Reset Bot? </th> <th>Operating System Notes</th>
</tr>
<tr>
<td>Upload Firmware with avrdude (no prior serial connection)</td><td>yes</td><td>works on all platforms</td>
</tr>
<tr>
<td>Connect to RepG 37</td><td> no</td><td>works on all platforms</td>
</tr>
<tr>
<td>Connect to Bot with s3g module @ 115200baud, query temperature, close</td><td>no</td><td>works on all platforms</td>
</tr>
<tr>
<td>Upload firmware with avrdude after connecting with RepG</td><td>yes</td><td> works on all platforms</td>
</tr>
<tr>
<td>Upload firmware with avrdude after connecting with s3g module @115200 baud</td><td>yes</td><td>works on all platforms</td>
</tr>
<tr>
<td>Upload firmware with avrdude two times in a row</td><td>yes</td><td>works on mac, linux, requires simple work around in windows -see next</td>
</tr>
<tr>
<td>Upload firmware with avrdude after sending avrdude connect request @1200 baud (any baud not 57600)</td><td>yes</td> <td>works on all platforms</td>
</tr>
<tr>
<td>Upload firmware with avrdude after connecting with pyserial @ 56700</td><td>yes</td><td> fails on all platforms, simple work around -see next</td>
</tr>
<tr>
<td>Upload firmware with avrdude after connecting with pyserial @ 1200 (any baud not 57600)</td><td>yes</td><td> works on all platforms</td>
</tr>
<tr>
<td>Upload firmware with avrdude after connecting with pyserial @ 57600, then setting s.baudrate=1200 (any baud not 57600)</td><td>yes</td><td>works on all platforms</td>
</tr>
</table>

Note that pyserial on all platforms and avrdude on windows do not set the baud rate on connect if the previous baud rate is deemed to be the same.  If the baud rate is not set, the reset will not be triggered.  A work around for this if a reset is desired is to connect @ 115200 (any baud different than 57600) before connecting at 57600 to upload firmware.
