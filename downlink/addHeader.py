#!/usr/bin/python

import sys
import struct
import os
import time
import hashlib
import subprocess

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
    #print fileSize
    #print fileHash
except IOError:
    print("Could not open file")
    sys.exit()

# create file hash
fileHashL = []
for i in range(0,32):
    fileHashL.append(ord(fileHash[i]))

# check if single digit file number
if(inFile[-6] == '_'):
    # single digit
    # extract zfec file number
    packNum = inFile[-7]
    # extract maximum number of zfec files
    nPackets = inFile[-5]
elif(inFile[-7]=='_'):
    # double digit
    # extract zfec file number
    packNum = int(inFile[-9:-7])
    # extract maximum number of zfec files
    nPackets = int(inFile[-6:-4])
elif(inFile[-8]=='_'):
    # triple digit
    # extract zfec file number
    packNum = int(inFile[-11:-8])
    # extract maximum number of zfec files
    nPackets = int(inFile[-7:-4])

# create control packet
# [packNum,nPackets,FileSize,FileSize,Hash,fileNameSize,fileName]
ctrlPacket = [packNum,nPackets,int((fileSize&0xFF00) >> 8),int(fileSize&0xFF)]
ctrlPacket.extend(fileHashL)
ctrlPacket.append(len(inFile))
ctrlPacket.extend(inFile)
print ctrlPacket

# append control packet to beginning of file
of.write(bytearray(ctrlPacket))
of.write(rawData)

# close files
inf.close()
of.close()

# remove fec file
subprocess.call(["rm",inFile])

# exit
sys.exit()
