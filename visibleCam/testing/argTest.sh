#!/bin/bash
# take one input jpg file and compress it to an output jpg file
# usage: compress [args] [infile] [outfile] 
# accepts:
# -q quality

while [[ $# -gt 1 ]]
do
key="$1"

case $key in
    -q|--quality)
    QUALITY="$2"
    shift # past argument
    ;;
    *)
        # unknown argument
    ;;
esac
shift
done
echo QUALITY = "${QUALITY}"
if [[ -n $1 ]]; then
    echo "Last line of file specified as non-opt/last argument:"
    tail -1 $1
fi
