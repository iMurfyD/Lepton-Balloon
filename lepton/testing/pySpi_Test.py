import io
import struct

fd = open('/dev/spidev0.0','rb')
nBytes = 164*60 
values = fd.read(nBytes)
processedVals = struct.unpack(">"+"B" *  nBytes,values)
print(processedVals)
fd.close()
