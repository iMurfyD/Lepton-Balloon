#!/bin/bash
# compresses one jpg file
# usage: ./compress [infile] [outfile] [quality]

djpeg $1 >> tempCompress.ppm
cjpeg -q $3 tempCompress.ppm  >> $2 
rm tempCompress.ppm
