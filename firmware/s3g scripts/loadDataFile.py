#!/usr/bin/python
#
# loads s3g files and prints them to tables that are readable by a microcontroller
# in a C format

# usage python loadDataFile.py [filename] [output table name]
# filename is a string representing the name of the file to open

import sys
import struct

f = open(sys.argv[1],'rb')

print "static uint8_t %s[] PROGMEM = {" % (sys.argv[2]) ,

dataCount = 0;
byte = f.read(1)
while byte:
	data = struct.unpack('>B', byte);
	byte = f.read(1)
	if byte:
		print "%s, " % (data) ,
	else:
		print "%s" % (data),
	dataCount += 1; 
	
	
print "};"

print "count: %s" % dataCount;

