@ECHO OFF
:startBurn
echo Ready to burn firmware for Motherboard v1.2.
pause

tools-win\avrdude -cstk500v1 -b57600 -D -v -D -pm1280 -PCOM1 -Uflash:w:MBv2.4/PRODUCTION.hex:i 

if errorlevel 1 (
echo *** FAILURE *** Failed to verify program.  Try again.
goto startBurn
)

if errorlevel 1 (
echo *** FAILURE *** Failed to verify program.  Try again.
goto startBurn
)

goto startBurn