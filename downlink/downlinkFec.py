#!/usr/bin/python

import smbus
import RPi.GPIO as GPIO
import time
import sys
import struct
import os
import time
import hashlib

# bytes to read/transmit at once
CHUNKSIZE = 32
PACKETSIZE = 228

# busy flag pin
BusyFlag = 4 # Broadcom pin 4
ArdReset = 27 # arduino reset line 

# check number of arguments
if len(sys.argv) != 2:
    print("Expected single argument")
    sys.exit()

# extract file name for syntactic clarity
fileName = sys.argv[1]

# get I2C bus
bus = smbus.SMBus(1)

# device address
ADDRESS = 0x0F

# open binary file
try:
    f = open(sys.argv[1],"rb")
    fileSize = os.path.getsize(fileName)
    fileHash = hashlib.md5(f.read()).hexdigest()
    print fileSize
    print fileHash
except IOError:
    print("Could not open file")
    sys.exit()

# create control packet
fileHashL = []
for i in range(1,32):
    fileHashL.append(ord(fileHash[i]))

ctrlPacket = [0,0,int((fileSize&0xFF00) >> 8),int(fileSize&0xFF)]
ctrlPacket.extend(fileHashL)
print ctrlPacket

# append control packet to beginning of file
f.write(bytearray(ctrlPacket))

# Pin Setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(BusyFlag, GPIO.IN, pull_up_down=GPIO.PUD_OFF)
GPIO.setup(ArdReset, GPIO.OUT)
GPIO.output(ArdReset, GPIO.HIGH)

# iterate through file by chunks until EOF
for chunk in iter(lambda: f.read(CHUNKSIZE), ''):
    # convert from string to int
    chunk = list(struct.unpack('B'*len(chunk),chunk))
    for n in range(0,len(chunk)):
        chunk[n] = int(chunk[n])
    # wait for busy flag to clear
    while(GPIO.input(BusyFlag)):
        time.sleep(0.01)
    # send out chunk over i2c
    try:
        bus.write_i2c_block_data(ADDRESS,len(chunk),chunk)
    except IOError:
        print("failed to write to Arduino")
        # close I2C bus
        bus.close()
        # reset arduino
        GPIO.output(ArdReset, GPIO.LOW)
        time.sleep(0.1)
        GPIO.output(ArdReset, GPIO.HIGH)
        time.sleep(3)
        # reopen I2C bus
        bus = smbus.SMBus(1)
        # move back to attempt to retransmit failed chunk
        f.seek(f.tell()-CHUNKSIZE)

# close file smbus and exit
f.close()
bus.close()
GPIO.cleanup()
sys.exit()
