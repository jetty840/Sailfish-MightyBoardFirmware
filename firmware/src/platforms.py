platforms = {

    'mighty_one' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'model' : 'REPLICATOR',
          'board_platform' : 'mighty_one',
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc', 'LiquidCrystalSerial.cc',
                        'DigiPots.cc', 'PSU.cc', 'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc' ]
          },

    'mighty_one-2560' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'model' : 'REPLICATOR',
          'board_platform' : 'mighty_one',
          'defines' : ['BUILD_STATS', 'ALTERNATE_UART']
          },

    'mighty_two' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'model' : 'REPLICATOR2',
          'board_platform' : 'mighty_two',
          'defines' : ['SINGLE_EXTRUDER', 'BUILD_STATS'],
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc', 'LiquidCrystalSerial.cc',
                        'DigiPots.cc', 'PSU.cc', 'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc', 'TemperatureTable.cc', 'Thermistor.cc', 'Thermocouple.cc',
                        'Heater.cc', 'CoolingFan.cc', 'PID.cc',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/lib_sd/*.c\') ]',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/boards/mighty_two/*.cc\') ]' ]
        },

    'mighty_two-2560' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'model' : 'REPLICATOR2',
          'board_platform' : 'mighty_two',
          'defines' : ['SINGLE_EXTRUDER', 'BUILD_STATS', 'ALTERNATE_UART']
          },

    'mighty_twox' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'model' : 'REPLICATOR2',
          'board_platform' : 'mighty_two',
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc', 'LiquidCrystalSerial.cc',
                        'DigiPots.cc', 'PSU.cc', 'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc', 'TemperatureTable.cc', 'Thermistor.cc', 'Thermocouple.cc',
                        'Heater.cc', 'CoolingFan.cc', 'PID.cc',
  '[ os.path.basename(f) for f in glob.glob(\'../../src/MightyBoard/Motherboard/boards/mighty_two/*.cc\') ]' ]
        },

    'mighty_twox-2560' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'model' : 'REPLICATOR2',
          'board_platform' : 'mighty_two',
          'defines' : ['BUILD_STATS']
        },

    'ff_creator' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'model' : 'REPLICATOR',
          'board_platform' : 'mighty_one',
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc', 'LiquidCrystalSerial.cc',
                        'DigiPots.cc', 'PSU.cc', 'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc' ]
        },

    'ff_creator-2560' :
        { 'mcu' : 'atmega2560', 
          'programmer' : 'stk500v2',
          'model' : 'REPLICATOR',
          'board_platform' : 'mighty_one',
          'defines' : ['BUILD_STATS']
        },

    'ff_creatorx-2560' :
        { 'mcu' : 'atmega2560',
          'programmer' : 'stk500v2',
          'model' : 'REPLICATOR',
          'board_platform' : 'mighty_one',
          'defines' : ['BUILD_STATS']
          },

    'wanhao_dup4' :
        { 'mcu' : 'atmega1280',
          'programmer' : 'stk500v1',
          'model' : 'REPLICATOR',
          'board_platform' : 'mighty_one',
          'squeeze' : [ 'Menu.cc', 'Interface.cc', 'InterfaceBoard.cc', 'LiquidCrystalSerial.cc',
                        'DigiPots.cc', 'PSU.cc', 'Eeprom.cc', 'PSU.cc', 'EepromMap.cc', 'Piezo.cc',
                        'UtilityScripts.cc' ]
        }
}

