@ECHO OFF
:startBurn
echo Ready to load bootloader for Extruder v3.6.
pause

set port=usb
set firmware=ECv3.6\ATmegaBOOT_168_ec3x.hex
set programmer=usbtiny
set part=m168

tools-win\avrdude -v -p%part% -P%port% -c%programmer% -e -Ulock:w:0x3F:m -Uefuse:w:0x00:m -Uhfuse:w:0xDD:m -Ulfuse:w:0xEE:m

tools-win\avrdude -v -p%part% -P%port% -c%programmer% -e -Uflash:w:%firmware%:i

if errorlevel 1 (
echo *** FAILURE *** Failed to verify program.  Try again.
)

goto startBurn
