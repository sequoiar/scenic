#!/bin/sh

if [ $# != 1 ];then
    echo "provide a port offset to $(basename $0)"
fi
OFFSET=$1
MIVILLE=./miville.py
if [ -x $MIVILLE ];then 
    ./miville.py -o $OFFSET --miville-home $HOME/.sropulpof/$OFFSET
fi
