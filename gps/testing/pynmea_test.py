import pynmea2
import serial
import sys

ser = serial.Serial('/dev/ttyAMA0',9600)

while 1:
    try:
        data = ser.readline()
        if (data.startswith("$GPGGA")):
            msg = pynmea2.parse(data)
            print msg.lat
            print msg.lon
            print msg.altitude
            print msg.timestamp
    except(KeyboardInterrupt):
        ser.close()
        sys.exit()
