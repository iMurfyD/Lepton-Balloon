#!/usr/bin/python

import sys
import struct
import os
import time
import hashlib

# bytes to read/transmit at once
CHUNKSIZE = 32
PACKETSIZE = 228

# check number of arguments
if len(sys.argv) != 3:
    print("Expected input and output file")
    sys.exit()

# extract file name for syntactic clarity
inFile = sys.argv[1]
outFile = sys.argv[2]

# open binary file
try:
    inf = open(inFile,"rb")
    of = open(outFile,"wb")
    fileSize = os.path.getsize(inFile)
    rawData = inf.read()
    fileHash = hashlib.md5(rawData).hexdigest()
    print fileSize
    print fileHash
except IOError:
    print("Could not open file")
    sys.exit()

# create control packet
fileHashL = []
for i in range(0,32):
    fileHashL.append(ord(fileHash[i]))

ctrlPacket = [0,0,int((fileSize&0xFF00) >> 8),int(fileSize&0xFF)]
ctrlPacket.extend(fileHashL)
print ctrlPacket

# append control packet to beginning of file
of.write(bytearray(ctrlPacket))
of.write(rawData)

# close file smbus and exit
inf.close()
of.close()
sys.exit()
