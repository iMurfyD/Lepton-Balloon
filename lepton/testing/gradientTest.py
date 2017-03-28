from colour import Color
import math
import sys

N_POINTS = 255
MAXVAL = 255

red = Color("red")
colors = list(red.range_to(Color("blue"),N_POINTS))

for i in range(1,N_POINTS):
    sys.stdout.write("%d" % (math.floor(colors[i].red*MAXVAL)))
    sys.stdout.write(',')
    sys.stdout.write("%d" % (math.floor(colors[i].green*MAXVAL)))
    sys.stdout.write(',')
    sys.stdout.write("%d" % (math.floor(colors[i].blue*MAXVAL)))
    sys.stdout.write(',')
