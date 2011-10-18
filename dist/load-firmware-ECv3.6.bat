@ECHO OFF
:startBurn
echo Ready to flash firmware onto ECv3.6.
pause

listports.py > serialPort
set /p port= < serialPort

set firmware=ECv3.6\EC-ecv34-v3.0.hex
set programmer=stk500v1
set baud=19200
set part=m168


echo Attempting to connect to port %port% 

tools-win\avrdude -c%programmer% -b%baud% -D -v -p%part% -P%port% -Uflash:w:%firmware%:i 

if errorlevel 1 (
echo *** FAILURE *** Failed to verify program.  Try again.
)

goto startBurn
