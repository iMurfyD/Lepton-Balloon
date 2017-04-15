#!/bin/bash

NAME=("rebuilt_$1")
echo $NAME

INFILES=("$1.*")

zunfec -o$NAME $INFILES 
rm $INFILES
