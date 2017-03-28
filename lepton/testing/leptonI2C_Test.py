# Interfaces to FLIR Lepton Module over I2C

import smbus
import time

# Subsystem IDs
AGC_ID = 0x01
SYS_ID = 0x02
VID_ID = 0x03
OEM_ID = 0x08

# Command IDs
GET = 0x00
SET = 0x01
RUN = 0x10

# use I2C bus 1
bus = smbus.SMBus(1)

# Lepton Address
LEP_ADDR = 0x2a

# initializes lepton
def initLepton():
    # wait for lepton I2C interface to come up
    time.sleep(1)
    # check if lepton is busy
    error = read_reg(0x02)
    # if it is poll until it isnt
    while error != 0x06:
        print "busy"
        error = read_reg(0x02)
        time.sleep(0.01)
    return

# reads contents of a single register
def read_reg(reg):
    reading = 0
    # request two bytes of data from register (one 16 bit register)
    data = bus.read_i2c_block_data(LEP_ADDR,reg,2)
    # turn STUPID PYTHON STUPIDITY into real data
    realData = (data[0] << 8) | data[1]
    # print out register contents
    print("reg0x{0} = 0x{1}".format(reg,realData))
    return realData 

# read data returned in data register and print to serial console
def read_data():
    error = poll_status()
    nBytes = read_reg(0x06)
    print("Payload Length: {0}".format(nBytes))
    # request nBytes bytes of data from data register
    data = bus.read_i2c_block_data(LEP_ADDR,0x08,nBytes)
    # print out data
    for i in data:
        print(i)
    return

# write data to data register (including data length)
def write_data(data, nBytes):
    # check for error
    error = poll_status()
    # write data length to data length register
    bus.write_i2c_block_data(LEP_ADDR,0x06,nBytes)
    # write nBytes of data to data register (0x08)
    bus.write_i2c_block_data(LEP_ADDR,0x08,data)
    return

# send command to lepton
def lepton_command(moduleID, commandID, command):
    # check for error
    error = poll_status()
    data = [0,0]
    # check if security bit needs to be set
    if moduleID == OEM_ID:
        # write top byte as OEM ID + Security bit
        data[0] = 0x48
    else:
        # write top byte as module ID
        data[0] = moduleID & 0x0f
    # write command as to 6 bits of lower byte and command as last two bits
    # command can be:
    # GET: 00
    # SET: 01
    # RUN: 10
    data[1] = ( ((commandID << 2) & 0xfc) | (command & 0x3))
    # write command to command register (0x04)
    bus.write_i2c_block_data(LEP_ADDR,0x04,data)
    return

# poll until success
def poll_status():
    # check for error
    error = read_reg(0x02)
    # if it is poll until it isnt
    while (error != 0x06):
        error = read_reg(0x02)
        if (error != 0 and error != 6):
            print("Lepton Error: {0}".format(error>>8))
        time.sleep(0.001)
    return 

if __name__ == "__main__":
    print "Lepton I2C Demo"
    initLepton()
    # get serial number
    print "Getting Serial Number"
    lepton_command(SYS_ID, 0x28 >> 2, GET)
    # read back data
    read_data()
