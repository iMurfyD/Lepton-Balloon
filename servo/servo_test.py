# Servo Control
import sys
import time
import wiringpi

# read config file
try:
    cfgFile = open("servo.cfg",'r')
    for line in cfgFile:
        if line[0:10] == "ServoDwell":
            dwell = line[11:-1]
            dwell = int(dwell)
        elif line[0:8] == "ServoMin":
            servoMax = line[9:-1]
            servoMax = int(servoMax)
        elif line[0:8] == "ServoMax":
            servoMin = line[9:-1]
            servoMin = int(servoMin)

except IOError:
    print "Could not open config file"
    sys.exit()

# close config file
cfgFile.close()

# check if dwell has been discovered in cfg file
if (not 'dwell' in locals()):
    print "Servo dwell not found in cfg file"
    sys.exit()

# check if dwell has been discovered in cfg file
if (not 'servoMax' in locals()):
    print "Servo max angle not found in cfg file"
    sys.exit()

# check if dwell has been discovered in cfg file
if (not 'servoMin' in locals()):
    print "Servo min angle not found in cfg file"
    sys.exit()

# use 'GPIO naming'
wiringpi.wiringPiSetupGpio()

# set pin 18 to be a PWM output
wiringpi.pinMode(18, wiringpi.GPIO.PWM_OUTPUT)
wiringpi.pinMode(17, wiringpi.GPIO.INPUT)

# set the PWM mode to milliseconds type
wiringpi.pwmSetMode(wiringpi.GPIO.PWM_MODE_MS)

# set up clock division
wiringpi.pwmSetClock(192)
wiringpi.pwmSetRange(2000)

delay_period = 0.01

# send servo to center
wiringpi.pwmWrite(18,120)

while True:
    try:
        wiringpi.pwmWrite(18,servoMax)
        time.sleep(dwell)
        wiringpi.pwmWrite(18,servoMin)
        time.sleep(dwell)
    except KeyboardInterrupt:
        # aparently you can't cleanup gpios. Great.
        sys.exit()


