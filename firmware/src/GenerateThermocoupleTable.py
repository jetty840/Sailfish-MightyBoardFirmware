#!/usr/bin/python
#
# this function generates a lookup table to be used with the ADS1118 ADC and temperature sensor from Texas Instruments
# the lookup table is in the format {ADC_READING, temperature}  where temperature is degrees celcius
# a standard lookup table for k-type thermocouples converts temperature to miliVolts, where miliVolts expresses
# the voltage difference accross the two leads.
# To convert miliVolts to ADC bit values, we use miliVolts / full_scale_miliVolts * 32768
# where 32768 is the maximum ADC read and full_scale_milivolts is the corresponding maximum voltage

# usage python GenerateThermocoupleTemps.py

import sys

# import values from standard k-type thermocouple table 
# this table is available from many online sources such as:
# http://www.pyromation.com/downloads/data/emfk_c.pdf

ThermocoupleTemperatureToMiliVolt = [
  {'degrees_celcius':-64, 'miliVolts':-2.382},
  {'degrees_celcius':-48, 'miliVolts':-1.818},
  {'degrees_celcius':-32, 'miliVolts':-1.231},
  {'degrees_celcius':-16, 'miliVolts':-0.624},
  {'degrees_celcius':0, 'miliVolts':0},
  {'degrees_celcius':16, 'miliVolts':0.637},
  {'degrees_celcius':32, 'miliVolts':1.285},
  {'degrees_celcius':48, 'miliVolts':1.941},
  {'degrees_celcius':64, 'miliVolts':2.502},
  {'degrees_celcius':80, 'miliVolts':3.267},
  {'degrees_celcius':96, 'miliVolts':3.931},
  {'degrees_celcius':112, 'miliVolts':4.591},
  {'degrees_celcius':128, 'miliVolts':5.247},
  {'degrees_celcius':144, 'miliVolts':5.896},
  {'degrees_celcius':160, 'miliVolts':6.540},
  {'degrees_celcius':176, 'miliVolts':7.180},
  {'degrees_celcius':192, 'miliVolts':7.819},
  {'degrees_celcius':208, 'miliVolts':8.458},
  {'degrees_celcius':224, 'miliVolts':9.101},
  {'degrees_celcius':240, 'miliVolts':9.747},
  {'degrees_celcius':256, 'miliVolts':10.398},
  {'degrees_celcius':272, 'miliVolts':11.053},
  {'degrees_celcius':288, 'miliVolts':11.712},
  {'degrees_celcius':304, 'miliVolts':12.374},
  {'degrees_celcius':336, 'miliVolts':13.707},
  {'degrees_celcius':352, 'miliVolts':14.377},
  {'degrees_celcius':368, 'miliVolts':15.049},
  {'degrees_celcius':384, 'miliVolts':15.722},
  {'degrees_celcius':400, 'miliVolts':16.820}
  ]

#voltage at full scale ADC reading from ADS1118 
full_scale_milivolts = 256

print "static uint16_t thermocouple_lookup[] PROGMEM = {" 

for calibration in ThermocoupleTemperatureToMiliVolt:
  print "{%d, %d}," % (calibration['miliVolts'] * 32768 / full_scale_milivolts, calibration['degrees_celcius'] ) 

print "};"

    

