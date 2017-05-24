#!/usr/bin/env python

import RPi.GPIO as GPIO
import time
import datetime
import sys

# heater pin
HeaterPin = 4 # broadcom pin 4

# hysterisis tracker
hystOn = 0

# read config file
try:
    cfgFile = open("/home/avery/GitRepos/Lepton-Balloon/heater/heater.cfg",'r')
    for line in cfgFile:
        if line[0:9] == "HeaterLog":
            log = line[10:-1]
        elif line[0:9] == "MS5637Log":
            msLog = line[10:-1]
        elif line[0:9] == "HeaterMin":
            minTemp = line[10:-1]
            minTemp = int(minTemp)
        elif line[0:10] == "HeaterHyst":
            hyst = line[11:-1]
            hyst = int(hyst)
        elif line[0:10] == "HeaterFreq":
            freq = line[11:-1]
            freq = float(freq)

except IOError:
    print "Could not open config file"
    sys.exit()

# close config file
cfgFile.close()

# check if logfile has been discovered in cfg file
if (not 'log' in locals()):
    print "Heater logfile not found in cfg file"
    sys.exit()

# try to open logfile for append
try:
    logFile = open(log,'a')
except IOError:
    print "Could not open Heater log file"
    sys.exit()

# close logfile
logFile.close()

# check if ms5637 logfile has been discovered in cfg file
if (not 'msLog' in locals()):
    print "ms5637 logfile not found in cfg file"
    sys.exit()

# try to open ms5637 logfile for read
try:
    mslogFile = open(msLog,'r')
except IOError:
    print "Could not open ms5637 log file"
    sys.exit()

# close logfile
mslogFile.close()

# check if min temp has been discovered in cfg file
if (not 'minTemp' in locals()):
    print "Heater minimum temperature not found in cfg file"
    sys.exit()

# check if hysterisis has been discovered in cfg file
if (not 'hyst' in locals()):
    print "Heater hysterisis not found in cfg file"
    sys.exit()

# check if frequency has been discovered in cfg file
if (not 'freq' in locals()):
    print "Heater frequency not found in cfg file"
    sys.exit()
else:
    if not freq == 0:
        delay = 1/freq

# pin setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(HeaterPin,GPIO.OUT)
GPIO.output(HeaterPin,GPIO.LOW)

while 1:
    try:
        # try to open ms5637 logfile for read
        try:
            mslogFile = open(msLog,'r')
            # get most recent line from logfile
            curLine = mslogFile.readline().split(',')
            # parse line as two floats
            curTemp = float(curLine[1])
            # close logfile
            mslogFile.close()
        except IOError:
            print "Could not open ms5637 log file"
        # try to open heater logfile for append
        try:
            logFile = open(log,'a')
        except:
            print "Uh oh..."
        # write time to logfile
        data = datetime.datetime.now().time().strftime("%H:%M:%S") + ','
        logFile.write(data)
        # if temp is less than threashold turn on heater
        if curTemp<minTemp:
            logFile.write("ON\n")
            GPIO.output(HeaterPin, GPIO.HIGH)
            hystOn = 1
        elif (hystOn == 1) and (curTemp<(minTemp+hyst)):
            logFile.write("HYST\n")
            GPIO.output(HeaterPin, GPIO.HIGH)
        elif curTemp>(minTemp+hyst):
            logFile.write("OFF\n")
            GPIO.output(HeaterPin, GPIO.LOW)
            hystOn = 0
        # delay a bit
        time.sleep(delay)

    except KeyboardInterrupt:
        GPIO.output(HeaterPin, GPIO.LOW)
        sys.exit()
        GPIO.cleanup()
