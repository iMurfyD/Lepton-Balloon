#!/usr/bin/python

import sys
import struct
import os
import time
import hashlib

# bytes to read/transmit at once
#CHUNKSIZE = 32
#PACKETSIZE = 224
# packNum, nPackets, FileSize, FileSize, Hash, fileNameSize
# 1,       1,        1,        1,        32,   1
HEADERSIZE = 37

# check number of arguments
if len(sys.argv) != 2:
    print("Expected only input file")
    sys.exit()

# extract file name for syntactic clarity
inFile = sys.argv[1]

# open binary file
try:
    inf = open(inFile,"rb")
    fileSize = os.path.getsize(inFile)
    header = inf.read(HEADERSIZE)
#    rawData = inf.read()
#    fileHash = hashlib.md5(rawData).hexdigest()
#    print (fileSize - HEADERSIZE)
#    print fileHash
except IOError:
    print("Could not open file")
    sys.exit()

# print header for debug purposes
#print header

# decode file name length
nameLen = ord(header[36])
if nameLen < 5:
    print "Incorrect name length!"
    print nameLen
    inf.close()
    sys.exit()

# get file name
fileName = inf.read(nameLen)

# get remainder of file
rawData = inf.read()
fileHash = hashlib.md5(rawData).hexdigest()

# get packet number
packetNum = ord(header[0])

# get number of packets
nPacket = ord(header[1])
if nPacket < packetNum:
    print "Incorrect packet numbers!"
    print packetNum
    print nPacket
    inf.close()
    sys.exit()

# get file size
fileSize_control = (ord(header[2]) << 8) | ord(header[3])
if fileSize_control != fileSize-HEADERSIZE-nameLen:
    print "Incorrect File Size!"
    print (fileSize - HEADERSIZE - nameLen)
    print fileSize_control
    inf.close()
    sys.exit()

# check file hash
fileHash_control = ""
for i in range(0,32):
    fileHash_control += header[i+4]
if(fileHash != fileHash_control):
    print "Incorrect Packet Hash!"
    print fileHash
    print fileHash_control
    inf.close()
    sys.exit()

# rewrite out file as infile - header
of = open(fileName,"wb")
of.write(rawData)

# close files and exit
inf.close()
of.close()
sys.exit()
