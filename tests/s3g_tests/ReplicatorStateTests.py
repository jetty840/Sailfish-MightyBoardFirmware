
"""
A suite of tests to be run on a replicator with the s3g python module.  These tests are broken down into several categories:
"""
import os, sys 
lib_path = os.path.abspath('s3g/')
sys.path.append(lib_path)

try:
    import unittest2 as unittest
except ImportError:
    import unittest

import optparse
import serial
import io
import struct
import array
import time
import s3g 
import random
import csv
from array import array
from types import *

INT32_MAX = 2147483647
eeprom_max_offset = 0x1A2

axis_length_offsets = {
  'x_axis':{'offset': 0x00,'type':'<I', 'good_data':[0, 20000], 'default':10685},
  'y_axis':{'offset': 0x04,'type':'<I', 'good_data':[0, 20000], 'default':6966},
  'z_axis':{'offset': 0x08,'type':'<I', 'good_data':[0, 100000], 'default':60000},
  'a_axis':{'offset': 0x0C,'type':'<I', 'good_data':[0,INT32_MAX], 'default':9267520},
  'b_axis':{'offset': 0x0F,'type':'<I', 'good_data':[0,INT32_MAX], 'default':9267520}
  }
eeprom_acceleration_offsets = {
  'active':{'offset': 0x00,'type':'<B', 'good_data':[0,2], 'default':1}, 
  'default_rate':{'offset': 0x02,'type':'<h', 'good_data':[0,3500], 'default':3000},
  'x_axis_rate':{'offset': 0x04,'type':'<h', 'good_data':[0,3500], 'default':3000},
  'y_axis_rate':{'offset': 0x06,'type':'<h', 'good_data':[0,3500], 'default':3000},
  'z_axis_rate':{'offset': 0x08,'type':'<h', 'good_data':[0,3500], 'default':3000},
  'a_axis_rate':{'offset': 0x0A,'type':'<h', 'good_data':[0,3500], 'default':3000},
  'b_axis_rate':{'offset': 0x0C,'type':'<h', 'good_data':[0,3500], 'default':3000},
  'x_axis_jerk':{'offset': 0x0E,'type':'<BB', 'good_data':[0,40], 'default':[20,0]},
  'y_axis_jerk':{'offset': 0x10,'type':'<BB', 'good_data':[0,40], 'default':[20,0]},
  'z_axis_jerk':{'offset': 0x12,'type':'<BB', 'good_data':[0,40], 'default':[1,0]},
  'a_axis_jerk':{'offset': 0x14,'type':'<BB', 'good_data':[0,20], 'default':[2,0]},
  'b_axis_jerk':{'offset': 0x16,'type':'<BB', 'good_data':[0,20], 'default':[2,0]},
  'minimum_speed':{'offset': 0x18,'type':'<h', 'good_data':[0,40], 'default':15},
  'defaults_flag':{'offset': 0x1A,'type':'<B', 'good_data':[0,255], 'default':0x05}
  }
build_time_offsets = {
  'hours': {'offset': 0x00,'type':'<H', 'good_data':[0,65535], 'default':65520},
  'minutes': {'offset': 0x02,'type':'<B', 'good_data':[0,60], 'default':58}
}
eeprom_map =[
  {'name':'acceleration_settings', 'offset':0x016E, 'variables':eeprom_acceleration_offsets},
  {'name':'axis_lengths', 'offset':0x018C, 'variables':axis_length_offsets},
  {'name':'first_boot_flag', 'offset':0x0156, 'variables':{'first_boot':{'offset':0,'type':'>B', 'good_data':[0,2], 'default':1}}},
  {'name':'total_build_time', 'offset':0x01A0, 'variables':build_time_offsets},
  ] 

class EepromStateTests(unittest.TestCase):

  def setUp(self):
    self.s3g = s3g.s3g()
    self.s3g.file = serial.Serial(options.serialPort, '115200', timeout=1)
    self.s3g.writer = s3g.Writer.StreamWriter(self.s3g.file)
    self.s3g.abort_immediately()
    time.sleep(5)
  
  def tearDown(self):
    self.s3g.file.close()
  
  def ReadEEpromVariable(self, map_dict, variable):
    """
    read a variable stored in eeprom
    @param name: dictionary value for eeprom_map 'name'
    @param variable: dictionary value for 'variable' sub set in eeprom_map dict
    """
    offset = map_dict['offset'] + map_dict['variables'][variable]['offset']
    data_type = map_dict['variables'][variable]['type']
    data = self.s3g.read_from_EEPROM(offset, struct.calcsize(data_type))
    data_bytes = array('B', data)
    data = struct.unpack(data_type,data_bytes)
    print [variable, data]
    return data

  def EEpromCheckForValidEntries(self):
    """
    This test prints out eerpom entries
    Additional eeprom checks may be added in the future
    """

    for field in eeprom_map:
      for var in field['variables']:
        info = field['variables'][var]
        data = self.ReadEEpromVariable(field, var)
        self.assertTrue(data[0] < info['good_data'][1])


  def test_EEpromTestResetToFactory(self):

    self.s3g.reset_to_factory()
    time.sleep(5)
    self.EEpromCheckForValidEntries()

  def test_EEpromTestFullReset(self):
    """
    """
    data = bytearray()
    data.append(0xFF)
    for i in range(0, eeprom_max_offset):
      self.s3g.write_to_EEPROM(i, data)

    self.s3g.reset()
    time.sleep(16)

    self.EEpromCheckForValidEntries()

  """
  def EEpromWriteInvalidValues(self):

    for field in eeprom_map:
      for var in field['variables']:
  """

  def test_EepromWriteValidValues(self):

    for field in eeprom_map:
      for var in field['variables']:
        info = field['variables'][var]
        # hack to account for data types with two fields
        if type(info['default']) is ListType:
          self.s3g.write_to_EEPROM(field['offset'] + info['offset'], struct.pack(info['type'], info['default'][0], info['default'][1]))
        else:
          self.s3g.write_to_EEPROM(field['offset'] + info['offset'], struct.pack(info['type'], info['default']))         
 
    self.EEpromCheckForValidEntries()

class StateTests(unittest.TestCase):

  def HeatingErrorTest(self):

    tool_temps = []
    heat_cycle = 0

    csv_writer = csv.writer(open(options.filename, 'wb'), delimiter = ',')

    print "\n"

    tool_num = 2
    if options.toolCount == "single":
      tool_num = 1


    while(heat_cycle < 50):

      for tool_index in range(0,tool_num):

        print "heat_cycle: %d" % (heat_cycle)   
        #randomize whether tool or platform is heated first
        tool_first = random.randint(0,1) 
        if tool_first is 0:
          self.s3g.SetToolheadTemperature(tool_index,225);
          self.s3g.SetPlatformTemperature(tool_index,110);
        else: 
          self.s3g.SetPlatformTemperature(tool_index,110);
          self.s3g.SetToolheadTemperature(tool_index,225);
        
        # move axes to simulate start.gcode  
        self.s3g.FindAxesMaximums(['x', 'y'], 300, 60)
        self.s3g.FindAxesMinimums(['z'], 200, 60)
        self.s3g.RecallHomePositions(['x', 'y', 'z', 'a', 'b'])

        AnchorLocation = [-110.5*94.1397, -74*94.1397, 150*400, 0, 0]
        self.s3g.QueueExtendedPoint(AnchorLocation, 200)

        start_time = time.time()
        finished = False
        while finished is False:
          tool_temps.append(self.s3g.GetToolheadTemperature(tool_index))
          csv_writer.writerow([time.time(), tool_temps[-1]])
          tool_status = self.s3g.GetToolStatus(tool_index)
          for error, status in tool_status.iteritems() : 
            if status is True:
              finished = True
              if error is not "ExtruderReady":
                print tool_status
                print "tool head %d fail" % (tool_index)
                if tool_first is True:
                  print "tool heated before platform"
                else:
                  print "tool heated after platform"
                print "elapsed time: %d" % (time.time() - start_time)
                print "heat cycles: %d" % (heat_cycle)
                plt.plot(tool_temps)
                plt.show()
                self.assertFalse(status)
          time.sleep(0.3)

        tool_temps.append(self.s3g.GetToolheadTemperature(tool_index)) 
        csv_writer.writerow([time.time(), tool_temps[-1]])
        print "time: %d   temp: %d   count: %d " % (time.time() - start_time, tool_temps[-1], len(tool_temps))

        self.s3g.SetToolheadTemperature(tool_index, 0)
        self.s3g.SetPlatformTemperature(tool_index, 0)

        # give the tool a random amount of time to cool
        cool_time = (float(random.randint(1,16))/2)  * 60
        start_time = time.time()
        print "cool time: %f minutes" % (cool_time/60)
        while time.time() - start_time < cool_time:
          tool_temps.append(self.s3g.GetToolheadTemperature(tool_index))
          csv_writer.writerow([time.time(), tool_temps[-1]])
          time.sleep(0.03)

        heat_cycle += 1
    plt.plot(tool_temps)
    plt.show()

   


if __name__ == '__main__':
  parser = optparse.OptionParser()
  parser.add_option("-p", "--port", dest="serialPort", default="/dev/ttyACM0")
  parser.add_option("-f", "--file", dest="filename", default="temp_data_back.csv")
  parser.add_option("-t", "--tool_count", dest="toolCount", default="dual")
  (options, args) = parser.parse_args()

  del sys.argv[1:]

  #tests = unittest.TestLoader().loadTestsFromTestCase(EepromStateTests)
  tests = unittest.TestSuite()
  tests.addTest(EepromStateTests('test_EepromWriteValidValues'))
  

  unittest.TextTestRunner(verbosity=2).run(tests)

