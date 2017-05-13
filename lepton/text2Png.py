import png
import sys
import codecs

WIDTH = 80
HEIGHT = 60

n = 2

buf = []

#with open('LeptonCImage0.txt','rb') as inFile:
#    for i in range(0,HEIGHT):
#        s = inFile.read(WIDTH)
#        buf.append(s.split(','))
#        for j in range(0,WIDTH):
#            print(int(buf[i][j]))
#            buf[i][j]=int(buf[i][j])
#
#    print buf
#png.from_array(frameBuffer,'L;14').save('converted/LeptonTxtImage%d.png'%(n))

#inFile = codecs.open('LeptonCImage0.txt', encoding='ascii')
#for line in inFile:
#    s = repr(line)
#    buf.append(s.split(','))
#print buf

inFile = open('LeptonCImage2.txt', 'r')
for i in range(0,HEIGHT):
    s = inFile.readline()
    buf.append(s.split(','))
    buf[i][0] = int(buf[i][0])
    del buf[i][-1]
    for j in range(1,WIDTH):
        buf[i][j] = int(buf[i][j][1:])
print buf
png.from_array(buf,'L;14').save('converted/LeptonTxtImage%d.png'%(n))
