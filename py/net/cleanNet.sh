#!/bin/bash

usage () 
{
    echo "Usage: cleanNet.sh <INTERFACE>"
}


if [ $# -lt 1 ]
then
    echo "Invalid arguments."
    usage
    exit
fi

if [ $1 = "--help" ]
then
    usage
    exit
fi

INTERFACE=$1

echo "restoring conditions on interface "${INTERFACE}

# run this script to restore clean network conditions
tc qdisc del dev ${INTERFACE} root

