@ECHO OFF
:startBurn
echo Ready to flash firmware onto MBv2.4-2560.
pause

listports.py > serialPort
set /p port= < serialPort

set firmware=MBv2.4-2560\MB-mb24-2560-v2.9.hex
set programmer=stk500v2
set baud=115200
set part=m2560


echo Attempting to connect to port %port% 

tools-win\avrdude -c%programmer% -b%baud% -D -v -p%part% -P%port% -Uflash:w:%firmware%:i 

if errorlevel 1 (
echo *** FAILURE *** Failed to verify program.  Try again.
)

goto startBurn
