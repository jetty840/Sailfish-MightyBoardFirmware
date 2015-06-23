# platforms.py -- build settings for each "standard" firmware build.
#
#   Individuals can extend the list of known platforms by supplying
#   the file ~/.sailfish_platforms.py containing additional entries
#   for the platforms{} dictionary.  In the following example, an
#   entry is defined for a platform named "franken-board"
#
#     % cat ~/.sailfish_platforms.py
#     platforms['franken-board'] = {
#         'mcu' : 'atmega2560',
#         'programmer' : 'stk500v2',
#         'board_directory' : 'mighty_one',
#         'defines' : [ 'CORE_XY', 'BUILD_STATS', 'ALTERNATE_UART',
#                       'HEATERS_ON_STEROIDS', 'MAX31855',
#                       '__DELAY_BACKWARD_COMPATIBLE__',
#                       '__PROG_TYPES_COMPAT__' ]
#     }
#
#  It can then be built with the simple command
#
#     % scons platform=franken-board
#
#  which would be largely equivalent to
#
#     % scons platform=mighty_one-2560 core_xy=1 heaters_on_steroids=1 \
#          alt_uart=1 max31855=1
#
#  It misses setting -DBUILD_STATS since there is no command line option
#  to select that.

from os.path import expanduser, isfile

platforms = {

# This is a dictionary of platform names to build.
# Each platform to build is itself a dictionary containing build settings.
# The settings are
#
#   mcu        -- Processor name (e.g., atmega1280)
#   programmer -- avrdude programmer type (e.g., stk500v1)
#   board_directory -- Name of the motherboard specific board directory to
#                      use under firmware/src/MightyBoard/Motherboard/boards/
#                      (e.g., mighty_one)
#   defines    -- List of #defines to establish.  Any string prefixed with '-'
#                 will be removed from the list of #defines to establish.
#   squeeze    -- Source files to compile --mcall-prologues so as to save
#                 code space.

    'mighty_one' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'board_directory' : 'mighty_one',
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc',
                        'LiquidCrystalSerial.cc', 'DigiPots.cc', 'PSU.cc',
                        'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc', 'RGB_LED.cc',
                        'StandardButtonArray.cc',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/boards/mighty_one/*.cc\') ]' ],
          'defines' : [ 'HAS_RGB_LED', 'EEPROM_MENU_ENABLE' ]
          },

    'mighty_one-corexy' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'board_directory' : 'mighty_one',
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc',
                        'LiquidCrystalSerial.cc', 'DigiPots.cc', 'PSU.cc',
                        'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/lib_sd/*.c\') ]',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/boards/mighty_one/*.cc\') ]' ],
          'defines' : [ 'CORE_XY', 'HEATERS_ON_STEROIDS', 'BUILD_STATS',
                        'COOLING_FAN_PWM', 'HAS_RGB_LED', 'EEPROM_MENU_ENABLE' ]
        },

    'mighty_one-2560' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_one',
          'defines' : [ 'BUILD_STATS', 'ALTERNATE_UART', 'AUTO_LEVEL',
                        'PSTOP_ZMIN_LEVEL', 'HAS_RGB_LED',
                        'EEPROM_MENU_ENABLE', 'RGB_LED_MENU' ]
        },

    'mighty_one-2560-corexy' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_one',
          'defines' : [ 'CORE_XY', 'BUILD_STATS', 'ALTERNATE_UART',
                        'HEATERS_ON_STEROIDS', 'AUTO_LEVEL', 'HAS_RGB_LED',
                        'PSTOP_ZMIN_LEVEL', 'COOLING_FAN_PWM',
                        'EEPROM_MENU_ENABLE', 'RGB_LED_MENU' ]
        },

    'mighty_one-2560-clone-r1' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_one',
          'defines' : [ 'CORE_XY', 'BUILD_STATS', 'ALTERNATE_UART',
                        'HEATERS_ON_STEROIDS', 'AUTO_LEVEL', 'HAS_RGB_LED',
                        'PSTOP_ZMIN_LEVEL', 'COOLING_FAN_PWM',
                        'EEPROM_MENU_ENABLE', 'CLONE_R1', 'RGB_LED_MENU' ]
        },

    'mighty_one-2560-max31855-corexy' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_one',
          'defines' : [ 'CORE_XY', 'BUILD_STATS', 'ALTERNATE_UART',
                        'HAS_RGB_LED', 'HEATERS_ON_STEROIDS', 'AUTO_LEVEL',
                        'MAX31855', 'PSTOP_ZMIN_LEVEL', 'COOLING_FAN_PWM',
                        'EEPROM_MENU_ENABLE', 'RGB_LED_MENU' ]
        },

    'mighty_one-2560-max31855' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_one',
          'defines' : [ 'BUILD_STATS', 'ALTERNATE_UART', 'MAX31855',
                        'AUTO_LEVEL', 'PSTOP_ZMIN_LEVEL', 'HAS_RGB_LED',
                        'EEPROM_MENU_ENABLE', 'RGB_LED_MENU' ]
        },

    'mighty_two' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'board_directory' : 'mighty_two',
          'defines' : [ 'SINGLE_EXTRUDER', 'BUILD_STATS', 'HAS_RGB_LED',
                        'EEPROM_MENU_ENABLE' ],
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc',
                        'LiquidCrystalSerial.cc', 'DigiPots.cc', 'PSU.cc',
                        'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc', 'TemperatureTable.cc',
                        'Thermistor.cc', 'Thermocouple.cc', 'Heater.cc',
                        'CoolingFan.cc', 'PID.cc',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/lib_sd/*.c\') ]',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/boards/mighty_two/*.cc\') ]' ]
        },

    'mighty_two-corexy' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'board_directory' : 'mighty_two',
          'defines' : [ 'CORE_XY', 'SINGLE_EXTRUDER', 'BUILD_STATS', 'HAS_RGB_LED',
                        'EEPROM_MENU_ENABLE' ],
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc',
                        'LiquidCrystalSerial.cc', 'DigiPots.cc', 'PSU.cc',
                        'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc', 'TemperatureTable.cc',
                        'Thermistor.cc', 'Thermocouple.cc', 'Heater.cc',
                        'CoolingFan.cc', 'PID.cc',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/lib_sd/*.c\') ]',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/boards/mighty_two/*.cc\') ]' ]
        },

    'mighty_two-2560' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_two',
          'defines' : [ 'SINGLE_EXTRUDER', 'BUILD_STATS', 'ALTERNATE_UART',
                        'AUTO_LEVEL', 'PSTOP_ZMIN_LEVEL', 'HAS_RGB_LED',
                        'EEPROM_MENU_ENABLE', 'RGB_LED_MENU' ]
        },

    'mighty_twox' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'board_directory' : 'mighty_two',
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc',
                        'LiquidCrystalSerial.cc', 'DigiPots.cc', 'PSU.cc',
                        'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc', 'TemperatureTable.cc',
                        'Thermistor.cc', 'Thermocouple.cc', 'Heater.cc',
                        'CoolingFan.cc', 'PID.cc',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/boards/mighty_two/*.cc\') ]' ],
          'defines' : [ 'BUILD_STATS', 'HAS_RGB_LED', 'EEPROM_MENU_ENABLE' ]
        },

    'mighty_twox-2560' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_two',
          'defines' : [ 'BUILD_STATS', 'ALTERNATE_UART', 'AUTO_LEVEL',
                        'PSTOP_ZMIN_LEVEL', 'HAS_RGB_LED',
                        'EEPROM_MENU_ENABLE', 'RGB_LED_MENU' ]
        },

    'ff_creator' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'board_directory' : 'mighty_one',
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc',
                        'LiquidCrystalSerial.cc', 'DigiPots.cc', 'PSU.cc',
                        'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc' ],
          'defines' :  [ 'FF_CREATOR', 'HEATERS_ON_STEROIDS', 'HAS_RGB_LED',
                         'EEPROM_MENU_ENABLE' ]
        },

    'ff_creator-2560' :
        { 'mcu' : 'atmega2560', 
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_one',
          'defines' : [ 'FF_CREATOR', 'BUILD_STATS', 'ALTERNATE_UART',
                        'HEATERS_ON_STEROIDS', 'AUTO_LEVEL',
                        'PSTOP_ZMIN_LEVEL', 'HAS_RGB_LED', 'RGB_LED_MENU',
                        'EEPROM_MENU_ENABLE' ]
        },

    'ff_creatorx-2560' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_one',
          'defines' : [ 'BUILD_STATS', 'ALTERNATE_UART', 'FF_CREATOR_X',
                        'HEATERS_ON_STEROIDS', 'AUTO_LEVEL',
                        'PSTOP_ZMIN_LEVEL', 'HAS_RGB_LED', 'RGB_LED_MENU',
                        'EEPROM_MENU_ENABLE' ]
        },

    'wanhao_dup4' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'board_directory' : 'mighty_one',
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc',
                        'LiquidCrystalSerial.cc','DigiPots.cc', 'PSU.cc',
                        'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc' ],
          'defines' : [ 'WANHAO_DUP4', 'HEATERS_ON_STEROIDS',
                        'HAS_RGB_LED', 'EEPROM_MENU_ENABLE' ]
        },

    'zyyx-1280' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'board_directory' : 'mighty_one',
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc',
                        'LiquidCrystalSerial.cc','DigiPots.cc', 'PSU.cc',
                        'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc' ],
          'defines' : [ 'SINGLE_EXTRUDER', 'ZYYX_3D_PRINTER',
                        'AUTO_LEVEL', 'AUTO_LEVEL_ZYYX', 'PSTOP_ZMIN_LEVEL' ]
        },

    'zyyx-2560' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_one',
          'defines' : [ 'EEPROM_MENU_ENABLE', 'BUILD_STATS', 'SINGLE_EXTRUDER',
                        'ALTERNATE_UART', 'ZYYX_3D_PRINTER',
                        'HEATERS_ON_STEROIDS', 'AUTO_LEVEL', 'AUTO_LEVEL_ZYYX',
                        'PSTOP_ZMIN_LEVEL', 'ZYYX_LEVEL_SCRIPT' ]
        },

    'zyyx-dual-2560' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'mighty_one',
          'defines' : [ 'EEPROM_MENU_ENABLE', 'BUILD_STATS',
                        'ALTERNATE_UART', 'ZYYX_3D_PRINTER',
                        'HEATERS_ON_STEROIDS', 'AUTO_LEVEL', 'AUTO_LEVEL_ZYYX',
                        'PSTOP_ZMIN_LEVEL', 'ZYYX_LEVEL_SCRIPT' ]
        },

    'azteeg-x3' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'azteeg_x3',
          'defines' : [ 'BUILD_STATS', 'HEATERS_ON_STEROIDS',
                        'AUTO_LEVEL', 'PSTOP_ZMIN_LEVEL', 'COOLING_FAN_PWM',
                        'EEPROM_MENU_ENABLE' ]
        },

    'azteeg-x3-corexy' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'board_directory' : 'azteeg_x3',
          'defines' : [ 'CORE_XY', 'BUILD_STATS', 'HEATERS_ON_STEROIDS',
                        'AUTO_LEVEL', 'PSTOP_ZMIN_LEVEL', 'COOLING_FAN_PWM',
                        'EEPROM_MENU_ENABLE' ]
        },
}

# Load data from ~/.sailfish_platforms.py

tmp_dict = { 'platforms' : {} }
home = expanduser('~')
if home[-1] != '/':
    home += '/'
site_file = home + '.sailfish_platforms.py'

if isfile(site_file):
    with open(site_file) as f:
        exec(f.read(), tmp_dict)

    if 'platforms' in tmp_dict:
        tmp_platforms = tmp_dict['platforms']
        for key in tmp_platforms:
            platforms[key] = tmp_platforms[key]

if __name__ == '__main__':
    list = ''
    for key in platforms:
        list += key + ' '
    print list[:-1]
