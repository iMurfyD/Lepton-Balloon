import pynmea2
import serial
import sys

ser = serial.Serial('/dev/ttyAMA0',9600)

# read config file
try:
    cfgFile = open("gps.cfg","r")
    for line in cfgFile:
        #print line[0:13]
        if line[0:13] == "GpsPrimaryLog":
            log1 = line[14:-1]
            #print log1
        elif line[0:15] == "GpsSecondaryLog":
            log2 = line[16:-1]
            #print log2
except IOError:
    print "Could not open Config File"
    ser.close()
    sys.exit()

# close config file
cfgFile.close()    

# check if logfiles have been discovered in cfg file
if (not 'log1' in locals() or not 'log2' in locals()):
    print "Gps logfiles not found in cfg file"
    ser.close()
    sys.exit()

# try to open logfiles for append + read
try:
    log1File = open(log1,'a+')
    log2File = open(log2,'a+')
except IOError:
    print "Could not open log files"
    ser.close()
    sys.exit()

# close logfiles
log1File.close()
log2File.close()

while 1:
    try:
        data = ser.readline()
        if (data.startswith("$GPGGA")):
            # try to open log file 1
            try:
                logFile = open(log1,'a+')
                print "logfile 1 open"
            except IOError:
                # try to open log file 2
                try:
                    logFile = open(log2,'a+')
                except IOError:
                    print "Uh oh..."
            # parse nmea data    
            msg = pynmea2.parse(data)
            # get lat lon alt and time
            print msg.lat
            print msg.lon
            print msg.altitude
            print msg.timestamp
            # concatenate into string
            data = msg.timestamp.strftime("%H:%M:%S") + ',' + str(msg.lat) + ',' + str(msg.lon) + ',' + str(msg.altitude) + '\n'
            # write data to file
            logFile.write(data)
            # close logfile
            logFile.close()
    except(KeyboardInterrupt):
        logFile.close()
        ser.close()
        sys.exit()
