#!/usr/bin/python

import RPi.GPIO as GPIO
import time
import sys

# heater pin
HeaterPin = 4 # broadcom pin 4

# pin setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(HeaterPin, GPIO.OUT)
GPIO.output(HeaterPin, GPIO.LOW)

while 1:
    try:
        time.sleep(1)
        GPIO.output(HeaterPin, GPIO.HIGH)
        time.sleep(1)
        GPIO.output(HeaterPin, GPIO.LOW)
    except KeyboardInterrupt:
        GPIO.output(HeaterPin, GPIO.LOW)
        GPIO.cleanup()
        sys.exit()

