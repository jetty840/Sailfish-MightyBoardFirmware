@ECHO OFF
:startBurn
echo Ready to burn firmware for Motherboard v1.2.
pause

tools-win\avrdude -v -pm168 -cusbtiny -e -Ulock:w:0x3F:m -Uefuse:w:0x00:m -Uhfuse:w:0xDD:m -Ulfuse:w:0xEE:m

tools-win\avrdude -v -pm168 -cusbtiny -e -Uflash:w:ECv3.6/PRODUCTION.hex

if errorlevel 1 (
echo *** FAILURE *** Failed to verify program.  Try again.
goto startBurn
)

if errorlevel 1 (
echo *** FAILURE *** Failed to verify program.  Try again.
goto startBurn
)

goto startBurn