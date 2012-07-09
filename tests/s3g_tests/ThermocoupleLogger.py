
"""
get thermocouple readings from the replicator with a time stamp so we can correlate them to an external reference
"""
import os, sys 
lib_path = os.path.abspath('../')
sys.path.append(lib_path)
lib_path = os.path.abspath('../s3g/')
sys.path.append(lib_path)


import optparse
import serial
import io
import struct
import array
import time
import s3g 
import random
import csv
import matplotlib.pyplot as plt
from coding import *
import time

s3g_port = s3g.s3g()

def GetThermocoupleReads():
  
  log_file = csv.writer(open(options.filename, 'wb'), delimiter=' ')
  start_time = time.time()

  while 1:
    try:
      raw_temp = s3g_port.get_toolhead_temperature(0)
      log_file.writerow([time.time()-start_time, raw_temp])
      time.sleep(0.245)
      print "channel 0 %d   %f" % (raw_temp, time.time()-start_time)
       
    except (KeyboardInterrupt) :
      return
      
def setUp():
  file = serial.Serial(options.serialPort, '115200', timeout=1)
  s3g_port.writer = s3g.Writer.StreamWriter(file)
  s3g_port.set_extended_position([0, 0, 0, 0, 0])
  s3g_port.abort_immediately()
  time.sleep(2)

def tearDown():
  s3g_port.writer.file.close()


if __name__ == '__main__':
  parser = optparse.OptionParser()
  parser.add_option("-p", "--port", dest="serialPort", default="/dev/ttyACM0")
  parser.add_option("-f", "--file", dest="filename", default="temp_data.csv")
  (options, args) = parser.parse_args()

  del sys.argv[1:]

  setUp();

  GetThermocoupleReads()
  tearDown()
    
