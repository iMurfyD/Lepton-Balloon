import png
from colour import Color
import math
import sys

WIDTH = 80
HEIGHT = 40
MAXVAL = 255

red = Color("red")
colors = list(red.range_to(Color("blue"),WIDTH*HEIGHT))

frame = []
row = []

#img = Image.new('RGB',(WIDTH,HEIGHT))
for j in range(0,HEIGHT):
    row = []
    for i in range(0,WIDTH):
        row.append( math.floor(colors[WIDTH*j+i].red*MAXVAL))
        row.append(math.floor(colors[WIDTH*j+i].green*MAXVAL))
        row.append(math.floor(colors[WIDTH*j+i].blue*MAXVAL))
    frame.append(row)
        #hex_data[WIDTH*j+i] = pix_color
        #print(hex_data)
print(frame)
png.from_array(frame,'RGB').save('gradient.png')
