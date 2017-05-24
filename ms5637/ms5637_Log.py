#!/usr/bin/env python
# modified code from controleverything.com
# Avery Bodenstein

import smbus
import time
import sys

convTime = 0.02

# read config file
try:
    cfgFile = open("/home/avery/GitRepos/Lepton-Balloon/ms5637/ms5637.cfg",'r')
    for line in cfgFile:
        if line[0:9] == "MS5637Log":
            log = line[10:-1]
        elif line[0:9] == "MS5637OSR":
            OSR = line[10:-1]
            OSR = int(OSR)
        elif line[0:10] == "MS5637Freq":
            freq = line[11:-1]
            freq = float(freq)

except IOError:
    print "Could not open config file"
    sys.exit()

# close config file
cfgFile.close()

# check if logfile has been discovered in cfg file
if (not 'log' in locals()):
    print "MS5637 Logfile not found in cfg file"
    sys.exit()

# try to open logfile for append
try:
    logFile = open(log,'a')
except IOError:
    print "Could not open MS5637 log file"
    sys.exit()

# close logfile
logFile.close()

# check if OSR is valid
if (not 'OSR' in locals()):
    print "no OSR defined in logfile"
    print "defaulting to 256"
    pressureCmd = 0x40
    tempCmd = 0x50
elif OSR == 256:
    pressureCmd = 0x40
    tempCmd = 0x50
elif OSR == 512:
    pressureCmd = 0x42
    tempCmd = 0x52
elif OSR == 1024:
    pressureCmd = 0x44
    tempCmd = 0x54
elif OSR == 2048:
    pressureCmd = 0x46
    tempCmd = 0x56
elif OSR == 4096:
    pressureCmd = 0x48
    tempCmd = 0x58
elif OSR == 8192:
    pressureCmd = 0x4A
    tempCmd = 0x5A
else:
    print "OSR not valid"
    print OSR
    pressureCmd = 0x40
    tempCmd = 0x50

# check if freq is valid
if (not 'freq' in locals()):
    print "no sampling Freqency defined in logfile"
    print "defaulting to 1 second"
    delay = 1.0
else:
    delay = 1/freq
# correct for expected conversion time delays and ensure positive delay
delay = delay - 2*convTime
if delay < 0:
    delay = 0
    
# Get I2C bus
bus = smbus.SMBus(1)

# MS5637_02BA03 address, 0x76(118)
#		0x1E(30)	Reset command
bus.write_byte(0x76, 0x1E)

time.sleep(0.5)

# Read 12 bytes of calibration data
# Read pressure sensitivity
data = bus.read_i2c_block_data(0x76, 0xA2, 2)
C1 = data[0] * 256 + data[1]

# Read pressure offset
data = bus.read_i2c_block_data(0x76, 0xA4, 2)
C2 = data[0] * 256 + data[1]

# Read temperature coefficient of pressure sensitivity
data = bus.read_i2c_block_data(0x76, 0xA6, 2)
C3 = data[0] * 256 + data[1]

# Read temperature coefficient of pressure offset
data = bus.read_i2c_block_data(0x76, 0xA8, 2)
C4 = data[0] * 256 + data[1]

# Read reference temperature
data = bus.read_i2c_block_data(0x76, 0xAA, 2)
C5 = data[0] * 256 + data[1]

# Read temperature coefficient of the temperature
data = bus.read_i2c_block_data(0x76, 0xAC, 2)
C6 = data[0] * 256 + data[1]

while 1:
    try:
        # MS5637_02BA03 address, 0x76(118)
        # begin pressure conversion using OSR defined in cfg file
        bus.write_byte(0x76, pressureCmd)

        time.sleep(convTime)

        # Read digital pressure value
        # Read data back from 0x00(0), 3 bytes
        # D1 MSB2, D1 MSB1, D1 LSB
        value = bus.read_i2c_block_data(0x76, 0x00, 3)
        D1 = value[0] * 65536 + value[1] * 256 + value[2]

        # MS5637_02BA03 address, 0x76(118)
        # begin temperature conversion using OSR defined in cfg file
        bus.write_byte(0x76, tempCmd)

        time.sleep(convTime)

        # Read digital temperature value
        # Read data back from 0x00(0), 3 bytes
        # D2 MSB2, D2 MSB1, D2 LSB
        value = bus.read_i2c_block_data(0x76, 0x00, 3)
        D2 = value[0] * 65536 + value[1] * 256 + value[2]

        # convert raw temp value to actual temperature
        dT = D2 - C5 * 256
        TEMP = 2000 + dT * C6 / 8388608
        OFF = C2 * 131072 + (C4 * dT) / 64
        SENS = C1 * 65536 + (C3 * dT ) / 128
        T2 = 0
        OFF2 = 0
        SENS2 = 0

        # compensate for nonlinearities at low temperature
        if TEMP > 2000 :
            T2 = 5 * dT * dT / 274877906944
            OFF2 = 0
            SENS2 = 0
        elif TEMP < 2000 :
            T2 = 3 * (dT * dT) / 8589934592
            OFF2 = 61 * ((TEMP - 2000) * (TEMP - 2000)) / 16
            SENS2 = 29 * ((TEMP - 2000) * (TEMP - 2000)) / 16
            if TEMP < -1500:
                OFF2 = OFF2 + 17 * ((Temp + 1500) * (Temp + 1500))
                SENS2 = SENS2 + 9 * ((Temp + 1500) * (Temp +1500))

        # convert raw pressure value to actual pressure
        TEMP = TEMP - T2
        OFF = OFF - OFF2
        SENS = SENS - SENS2
        pressure = ((((D1 * SENS) / 2097152) - OFF) / 32768.0) / 100.0
        cTemp = TEMP / 100.0

        # try to open logfile
        try:
            logFile = open(log,'a')
        except IOError:
            print "Uh oh..."
        # concatenate data into string
        data = time.strftime("%H:%M:%S") + ',' +  str(round(pressure,3)) + ','  + str(cTemp) + '\n'
        # Output data to logfile 
        logFile.write(data)
        # close logfile
        logFile.close()
        print "Pressure : %.2f mbar" %pressure
        print "Temperature in Celsius : %.2f C" %cTemp
        time.sleep(delay)
        
    except KeyboardInterrupt:
        logFile.close()
        sys.exit()
