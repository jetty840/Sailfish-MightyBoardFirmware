
#Timing Results

Reaing a block (512) bytes from the SD card takes ~2.5ms.    
Repeating this read and checking for equality adds ~3ms  

So we are doubling+ our read time by using this stability check.   
This is still much faster than the host serial interface, which takes on average 4ms to transmit 1 command


LCD Screen Behavior
It takes ~83ms to reset the LCD.  This is really too long to do regularly.  
We looked at refreshing the screen eg every 20s, but the reset is visible to the user, causeing visible flickering in the screen.  
It is preferable to have occasional screen garbling than constant flickering.  
We expect the screen garbling will be even less frequent with the proposed hardware fixes.  

Long file names: 
file name length 27 displays but throws and error when trying to print ("SD card read error")
file name length 28 does not display
