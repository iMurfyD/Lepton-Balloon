import smbus
import time

# get I2C bus
bus = smbus.SMBus(1)

# device address
ADDRESS = 0x0F

# write message
message = "Hello Avery!"
data = []
for c in message:
    data.append(ord(c))
bus.write_i2c_block_data(ADDRESS,len(message),data)
