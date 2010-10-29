@ECHO OFF
:startBurn
echo Ready to burn firmware for Motherboard v1.2.
pause

tools-win\avrdude -v -pm644p -cusbtiny -e -Ulock:w:0x3F:m -Uefuse:w:0xFD:m -Uhfuse:w:0xDC:m -Ulfuse:w:0xFF:m

tools-win\avrdude -v -pm644p -cusbtiny -e -Ulock:w:0x3F:m -U flash:w:MBv1.2/PRODUCTION.hex

if errorlevel 1 (
echo *** FAILURE *** Failed to verify program.  Try again.
goto startBurn
)

if errorlevel 1 (
echo *** FAILURE *** Failed to verify program.  Try again.
goto startBurn
)

goto startBurn