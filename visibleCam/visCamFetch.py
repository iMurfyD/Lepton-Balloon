#!/usr/bin/env python

import sys
import time
import datetime
import subprocess
import os
import glob

# approximate processing delays
picDelay = 0.75
compressDelay = 4.5

# read config file
try:
    cfgFile = open("/home/avery/GitRepos/Lepton-Balloon/visibleCam/visCam.cfg","r")
    for line in cfgFile:
        if line[0:11] == "VisCamDelay":
            delay = line[12:-1]
            delay = float(delay)
        elif line[0:10] == "VisCamQual":
            quality = line[11:-1]
            quality = int(quality)

except IOError:
    print "Could not open Config File"
    sys.exit()

# close config file
cfgFile.close()

# check if frequency has been discovered in cfg file
if (not 'delay' in locals()):
    print "Visible Camera delay not found in cfg file"
    sys.exit()
else:
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
        # get most recent jpg
        fileName = max(glob.iglob('/balloonLogs/*.jpg'), key=os.path.getctime)
        fileName = fileName[13:]
        print fileName
        baseName = max(glob.iglob('/balloonLogs/*.png'), key=os.path.getctime)
        baseName = baseName[13:]
        mlxName = baseName[:-7]+"MLX.png"
        lepName = baseName[:-7]+"lep.png"
        print lepName
        print mlxName
        # move MLX and Lep images
        subprocess.call(["cp","/balloonLogs/"+mlxName,"/downlinkStaging/" + mlxName + ".tmp"])
        subprocess.call(["cp","/balloonLogs/"+lepName,"/downlinkStaging/" + lepName + ".tmp"])
        # crop image before compressing
        subprocess.call(["/home/avery/GitRepos/Lepton-Balloon/visibleCam/crop.sh","/balloonLogs/"+fileName,"/balloonLogs/"+fileName+".tmp"])
        print "Cropped"
        # compress image
        subprocess.call(["/home/avery/GitRepos/Lepton-Balloon/visibleCam/compress.sh","/balloonLogs/"+fileName+".tmp","/balloonLogs/"+fileName+".cmp",str(quality)])
        print "Compressed"
        # move image to downlink folder
        subprocess.call(["cp","/balloonLogs/"+fileName+".cmp","/downlinkStaging/" + fileName + ".tmp"])
        # remove temp file
        subprocess.call(["rm","/balloonLogs/"+fileName+".tmp"])
        subprocess.call(["rm","/balloonLogs/"+fileName+".cmp"])
        print "removed"
        # delay
        time.sleep(delay)

        
    except KeyboardInterrupt:
        sys.exit()

