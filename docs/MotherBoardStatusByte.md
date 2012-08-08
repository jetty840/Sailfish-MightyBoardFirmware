#Motherboard Status Bytes
When are they turned on and off?

## Heater Inactive Shutdown
Indicates that heaters have been shutoff due to bot inactivity

__set true__:   
    on user inactivity timeout  

__set false__:   
    when any set_temperature command is processed with a temp > 0

## Onboard Script
Indicates the bot is currently running an onboard script.  The Onboard scripts include, the startup script, 
filament load/unload, level plate script, nozzle calibration script and home axes.

__set true__:  
    when a utility script start is processed  
    when the startup script, or filament load menu systems are entered  

__set false__:  
    when the host slice detects that the utility script is finished  
    when a build cancel is processed  
    when the filament load or startup scripts exit  

## Manual Mode
Indicates that the user is using the onboard stepper jog panel

__set true__:   
    when jog mode is entered (when screen updates)  

__set false__:  
    when jog mode is exited (when user backs out to previous menu)  

## Preheat
Indicates that the user has started a preheat via the onboard menu

__set true__:   
    when StartPreheat! is selected  

__set false__:   
    when Cool! is selected  
    when any set_temperature command is processed through the command buffer  
    if a user inactivity timeout occurs  
