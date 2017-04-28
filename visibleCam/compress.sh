#!/bin/bash
# compresses one jpg file
# usage: ./compress [infile] [outfile] [quality]

djpeg $1 | cjpeg -q $3 >> $2 
