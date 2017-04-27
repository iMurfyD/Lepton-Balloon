#!/bin/env python

import sys
import time
import datetime
import subprocess

# temp variables
quality = 30

# read config file
try:
    cfgFile = open("visCam.cfg","r")
    for line in cfgFile:
        #print line[0:13]
        if line[0:9] == "VisCamLog":
            log = line[10:-1]
except IOError:
    print "Could not open Config File"
    sys.exit()

# close config file
cfgFile.close()

# check if logfile has been discovered in cfg file
if (not 'log' in locals()):
    print "Visible Camera logfile not found in cfg file"
    sys.exit()

# try to open logfile for append
try:
    logFile = open(log,'a')
except IOError:
    print "Could not open log file"
    sys.exit()

# close logfile
logFile.close()

while 1:
    try:
        # get filename
        fileName = datetime.datetime.now().time().strftime("%d-%m-%y_%H:%M:%S")
        fileName = str(fileName) + ".jpg"
        print fileName
        # capture image from pycam
        subprocess.call("./capture.sh")
        # compress image from pycam
        subprocess.call(["./compress.sh","tempCapture.jpg",fileName,str(quality)])
        # remove temp file
        subprocess.call(["rm","tempCapture.jpg"])
        # delay 1 second
        time.sleep(1)
        
    except KeyboardInterrupt:
        # remove temp file
        subprocess.call(["rm","tempCapture.jpg"])
        #logFile.close()
        sys.exit()

