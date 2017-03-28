import time
import sys
import spidev
import png
import math

# spi speed
SPI_SPD = 20000000
# delay between word transfers us
SPI_DELAY = 0
# bits per word
SPI_BPW = 8
# VoSPI Packet length
PACKET_LEN = 164
# Number of VoSPI Packets per video frame
FRAME_PACKETS = 60

# instantiate spi object
spi = spidev.SpiDev()
# open port 0, CS 0
spi.open(0,0)
# max speed 20MHz
spi.max_speed_hz = 20000000
# spi mode 3
spi.mode = 0b11

def sync():
    print "syncing..."
    # de-assert CS
    # wait > 5 frame periods
    time.sleep(0.2)
    # read packets until a new frame is ready
    while True:
        # get header
        packet = spi.xfer2([0]*PACKET_LEN)
        #packet = spi.xfer2(row)
        header = int(packet[0])
        print(hex(header))
        # check if header is from a discard packet
        if(header & 0x0f) != 0x0f:
            return packet
    # lepton is now synced
    return packet

def getFrame(packet):
    # clear frameBuffer
    frameBuffer = []
    frameBuffer.append(packet)
    # create row primitive 
    #row = [0]*PACKET_LEN
    # get row
    for j in range(0,FRAME_PACKETS-1):
        # clear row buffer
        #rowBuffer = []
        # get each pixel
        #rowBuffer = spi.xfer2(row)
        # append row onto frame
        #frameBuffer.append(rowBuffer)
        frameBuffer.append(spi.xfer2([0]*PACKET_LEN))
    return frameBuffer

# turns byte stread into uint16 stream
def pack(frameBuffer):
    packBuffer = []
    for j in range(0,FRAME_PACKETS):
        # throw out header (maybe do CRC?)
        # clear tempBuf
        tempBuf = [0]*((PACKET_LEN-4)/2)
        for i in range(0,PACKET_LEN-4,2):
            # concatenate bytes
            #tempBuf[i/2] = (frameBuffer[j][i+5] << 6 | frameBuffer[j][i+4])&0x3fff
            tempBuf[i/2] = (frameBuffer[j][i+4] << 8 | frameBuffer[j][i+5])&0x3fff
            #tempBuf[i/2] = ((frameBuffer[j][i+4] & 0xf0 ) << 4 | frameBuffer[j][i+4])&0x3ff
        packBuffer.append(tempBuf)
    return packBuffer

# polls lepton packets until one is non-throwaway
def pollPacket():
    while True:
        # get header
        packet = spi.xfer2([0]*PACKET_LEN)
        #packet = spi.xfer2(row)
        header = int(packet[0])
#        print(hex(header))
        # check if header is from a discard packet
        if(header & 0x0f) != 0x0f:
            return packet
    # lepton is now synced
    return packet

if __name__ == "__main__":
    # main loop
    n = 0
    packet = sync()
    frameBuffer = getFrame(packet)
    frameBuffer = pack(frameBuffer)
    png.from_array(frameBuffer,'L;14').save('images/LeptonImage%d.png'%(n))
    while True:
        try:
            n = n+1
            # sync lepton with pi
            packet = pollPacket() 
            frameBuffer = getFrame(packet)
            frameBuffer = pack(frameBuffer)
            png.from_array(frameBuffer,'L;14').save('images/LeptonImage%d.png'%(n))
#            print(frameBuffer)
            print "butts"
        except KeyboardInterrupt:
            print "\nend"
            spi.close()
            sys.exit(0)
