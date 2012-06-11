#!/usr/bin/python
#
# generates a table of inverse feedrates, to be used in acceleration algorithm

# usage python GenerateFeedrateTable.py ['slow' / 'fast' / 'full']

import sys

if sys.argv[1] == 'slow':

	print "static uint16_t rate_table_slow[] PROGMEM = {" ,

	# values less than 32 clamp to zero (keeps results in uin16_t range)
	for i in range(0,32):
		print "0, ",

	for i in range (32,8191):
		print "%d, " % (1000000 / i),

	# last array member
	print "%d};" % (1000000 / 8191)

elif sys.argv[1] == 'fast':

	print "static uint16_t rate_table_fast[] PROGMEM = {" ,

	for i in range(8192,32768,16):
		print "%d, " % (int(round(1000000.0/float(i)))),

	# last array member
	print "%d};" % (1000000 / 32752)

if sys.argv[1] == 'full_slow':

	print "static uint16_t rate_table_slow[] PROGMEM = {" ,

	# values less than 32 clamp to zero (keeps results in uin16_t range)
	for i in range(0,32):
		print "0, ",

	for i in range (32,4906):
		print "%d, " % (1000000 / i),

	# last array member
	print "%d};" % (1000000 / 3907)

if sys.argv[1] == 'full_fast':

	print "static uint8_t rate_table_fast[] PROGMEM = {" ,

	for i in range (4906, 18839):
		print "%d, " % (1000000 / i),

	# last array member
	print "%d};" % (1000000 / 18839)


