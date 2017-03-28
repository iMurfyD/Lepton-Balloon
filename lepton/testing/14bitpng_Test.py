import png
import math
import sys

WIDTH = 80
HEIGHT = 40
MAXVAL = 16384

spacing = MAXVAL/(HEIGHT*WIDTH)

values = range(0,MAXVAL,spacing)

frame = []
row = []

for j in range(0,HEIGHT):
    row = []
    for i in range(0,WIDTH):
        row.append(math.floor(values[WIDTH*j+i]))
    frame.append(row)
print(frame)
png.from_array(frame,'L;14').save('14bit.png')
