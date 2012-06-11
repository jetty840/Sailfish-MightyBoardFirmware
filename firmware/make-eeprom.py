#!/usr/bin/env python

# Eeprom generator.  Takes a CSV, where each line is of the format:
# address,type,value
# Address is a 16-bit hex value.
# Type is one of the following:
#   u8   unsigned 8-bit int
#   u16  unsigned 16-bit int
#   f8.8 8.8 fixed point
# The value can be any integer or float value recognized by python.

import sys

size = 0x200

buffer = [0xff]*size
for line in sys.stdin.readlines():
    if line.startswith("#"):
        continue
    fields = line.strip().split(",")
    if len(fields) == 3:
        [addrstr,typestr,valstr] = fields;
        addr = int(addrstr,16)
        val = eval(valstr)
        if typestr == "u8":
            buffer[addr] = val;
        elif typestr == "u16":
            buffer[addr] = val & 0xff
            buffer[addr+1] = (val >> 8) & 0xff
        elif typestr == "f8.8":
            buffer[addr] = int(val) & 0xff
            buffer[addr+1] = int(val*256) & 0xff

# EEPROM GENERATION

offset = 0
blocksize = 16

while size > 0:
    codes = []
    linesize = min(blocksize,size)
    codes.append(linesize)
    codes.append(offset>>8)
    codes.append(offset&0xff)
    codes.append(0) # data code
    codes.extend(buffer[offset:offset+linesize])
    # calculate checksum
    checksum=(0x100-(sum(codes)&0xff))&0xff
    codes.append(checksum)
    print reduce(lambda s,v:s+'{0:02X}'.format(v), codes, ":")
    offset += linesize
    size -= linesize
