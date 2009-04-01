#!/bin/bash
# run this script as root to create 150ms delay conditions between SOURCE_IP an DEST IP
#
usage () 
{
    echo "Usage: sudo $(basename $0) <INTERFACE> <SOURCE_HOST> <DEST_HOST> <DELAY_USEC>"
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

if [ $# -lt 4 ]
then
    echo "Invalid arguments."
    usage
    exit
fi

INTERFACE=$1
SOURCE_HOST=$2
DEST_HOST=$3
DELAY_USEC=$4

echo "Delaying connection between src ${SOURCE_HOST} and dest ${DEST_HOST} on interface ${INTERFACE}"

# clean up
tc qdisc del dev ${INTERFACE} root
# root node
tc qdisc add dev ${INTERFACE} root handle 1: prio bands 10
# netem qdisc
tc qdisc add dev ${INTERFACE} parent 1:1 handle 10: netem delay ${DELAY_USEC}
tc filter add dev ${INTERFACE} protocol ip parent 1:0 prio 1 u32 match ip src ${SOURCE_HOST}/32 match ip dst ${DEST_HOST}/32 flowid 10:1
# other qdisc
tc qdisc add dev ${INTERFACE} parent 1:2 handle 20: pfifo
tc filter add dev ${INTERFACE} protocol ip parent 1:0 prio 2 u32 match ip src 0.0.0.0/0 match ip dst 0.0.0.0/0 flowid 20:2

