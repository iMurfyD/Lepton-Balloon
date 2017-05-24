#!/usr/bin/env python

import sys
import time
import datetime
import subprocess

# manually checked approximate processing delays for frequency calculation
picDelay = 0.75
compressDelay = 4.5

# read config file
try:
    cfgFile = open("/home/avery/GitRepos/Lepton-Balloon/visibleCam/visCam.cfg","r")
    for line in cfgFile:
        #print line[0:13]
        if line[0:10] == "VisCamName":
            name = line[11:-1]
        elif line[0:10] == "VisCamFreq":
            freq = line[11:-1]
            freq = float(freq)
        elif line[0:10] == "VisCamQual":
            quality = line[11:-1]
            quality = int(quality)

except IOError:
    print "Could not open Config File"
    sys.exit()

# close config file
cfgFile.close()

# check if picture name has been discovered in cfg file
if (not 'name' in locals()):
    print "Visible Camera file name not found in cfg file"
    sys.exit()

# check if frequency has been discovered in cfg file
if (not 'freq' in locals()):
    print "Visible Camera frequency not found in cfg file"
    sys.exit()
else:
    if freq != 0:
        delay = 1/freq
    else:
        delay = 0
    # add correction for camera capture time
    delay = delay - picDelay - compressDelay
    # ensure delay is never negative
    if delay < 0:
        delay = 0

# check if quality has been discovered in cfg file
if (not 'quality' in locals()):
    print "Visible Camera quality not found in cfg file"
    sys.exit()

while 1:
    try:
        # get filename
        fileName = "/balloonLogs/" + datetime.datetime.now().time().strftime("%H.%M.%S")
        fileName = str(fileName) + '_' + name + ".jpg"
        print fileName
        # capture image from pycam
        subprocess.call("./capture.sh")
        # crop image before compressing
        subprocess.call(["./crop.sh","tempCapture.jpg","tempCapture.jpg"])
        # compress image from pycam
        subprocess.call(["./compress.sh","tempCapture.jpg",fileName,str(quality)])
        # remove temp file
        subprocess.call(["rm","tempCapture.jpg"])
        # delay 1 second
        time.sleep(0.25)
        
    except KeyboardInterrupt:
        # remove temp file
        subprocess.call(["rm","tempCapture.jpg"])
        sys.exit()

