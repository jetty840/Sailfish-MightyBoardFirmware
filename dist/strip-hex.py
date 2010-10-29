#!/usr/bin/python

# Written by phooky at Makerbot Industries.
# Released into the public domain on account of being so dumb.

import sys

# Strips out all lines that are composed only of
# unprogrammed bits.
for line in sys.stdin.readlines():
    if line[0] != ':':
        sys.stdout.write(line)
        continue
    if line[7:9] != '00':
        sys.stdout.write(line)
        continue
    size = int(line[1:3],16)
    data = line[9:9+size*2]
    empty = True
    for c in data:
        if c != 'f' and c != 'F':
            empty = False
            break
    if not empty:
        sys.stdout.write(line)

