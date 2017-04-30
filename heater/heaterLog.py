import time
import sys

# read config file
try:
    cfgFile = open("heater.cfg",'r')
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

# try to open logfile for append
try:
    mslogFile = open(msLog,'a')
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

