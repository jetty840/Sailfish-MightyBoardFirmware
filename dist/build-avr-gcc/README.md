Cygwin does not work now for avr-gcc-4.6.2.

	   %  ../../gcc/doc/cppopts.texi:772: @itemx must follow @item
	   %  Makefile:4270: recipe for target 'doc/cpp.info' failed

The other thing is, that scons doesn't copy the header files to the build path.

Debian build is tested with Debian Version 7.5 (wheezy). You need to be a
sudo user.

	 % sudo visudo -f /etc/sudoers.d/username
	 username	ALL=(ALL) ALL

In the above `username` is your login name.

Ubuntu
------

Ubuntu build is rewritten and tested with Ubuntu 18.04 Server clean install. All dependencies are then installed by the build-avr-gcc-ubuntu script.
The build is updated for BINUTIL 2.32, GCC 8.2, AVRLIB 2.0. These are the recent in beginning 2019. All is tested to build correctly.
All warnings, except one, which i (Joe) do not understand, have been mitigated in the source in a way that it compiles with the recent and also still with older Compilers.

How to get up and running from scratch:

    1. Install Ubuntu Server 18.04, just the defaults, 2CPU, 4GB RAM, 40 GB Disk are fair,
       when finished log in with your install user.
    2. in your home directoy:  git clone https://github.com/<the repo you selected> <a local directory>
    3. change to your new and now populated directory
    4. execute  dist/build-avr-gcc/build-avr-gcc-ubuntu.sh
        4a. this will run long or longer, depending on your resources
        4b. you will be asked for your password several times for sudo
        4c. it will download, install and compile all required gear
    5. create the  ~/.sailfish_platforms.py file and populate it.
       More info can be found in the comments in  firmware/src/platforms.py
       For a "CTC Replicator 1" for example:

       platforms = {
         'my_mighty_one' :
            { 'mcu' : 'atmega1280',
              'programmer' : 'stk500v1',
              'board_directory' : 'mighty_one',
              'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc',
                            'LiquidCrystalSerial.cc', 'DigiPots.cc',
                            'Eeprom.cc', 'EepromMap.cc', 'Piezo.cc',
                            'UtilityScripts.cc', 'RGB_LED.cc',
                            'StandardButtonArray.cc',
          '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/boards/mighty_one/*.cc\') ]' ],
              'defines' : [ 'EEPROM_MENU_ENABLE', 'COOLING_FAN_PWM',
                            'HEATERS_ON_STEROIDS','BUILD_STATS',
                            'PLATFORM_SPLASH1_MSG=\\\"   CTC 3D Printer   \\\"',
                            'PLATFORM_THE_REPLICATOR_STR=\\\"Replicator 1\\\"']
            }
         }

    6. change to the firmware directory
    7. start the build by  scons platform=my_migthy_one
    8. not long after you will find the uploadable hex file in  build/my_mighty_one

