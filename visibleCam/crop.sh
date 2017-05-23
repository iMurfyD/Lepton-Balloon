#!/bin/bash
# crops a single jpg to lepton FOV
# usage ./crop.sh [infile] [outfile]

convert -crop 1500x1000+546+472 $1 $2
