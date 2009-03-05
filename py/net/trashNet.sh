#!/bin/bash
# run this script as root to create 5% packet loss conditions between SOURCE_IP an DEST IP
#
usage () 
{
    echo "Usage: sudo trashNet.sh <INTERFACE> <SOURCE_HOST> <DEST_HOST>"
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

if [ $# -lt 3 ]
then
    echo "Invalid arguments."
    usage
    exit
fi

INTERFACE=$1
SOURCE_HOST=$2
DEST_HOST=$3

echo "Trashing connection between src ${SOURCE_HOST} and dest ${DEST_HOST} on interface ${INTERFACE}"

# clean up
tc qdisc del dev ${INTERFACE} root
# root node
tc qdisc add dev ${INTERFACE} root handle 1: prio bands 10
# netem qdisc
tc qdisc add dev ${INTERFACE} parent 1:1 handle 10: netem loss 5%
tc filter add dev ${INTERFACE} protocol ip parent 1:0 prio 1 u32 match ip src ${SOURCE_HOST}/32 match ip dst ${DEST_HOST}/32 flowid 10:1
# other qdisc
tc qdisc add dev ${INTERFACE} parent 1:2 handle 20: pfifo
tc filter add dev ${INTERFACE} protocol ip parent 1:0 prio 2 u32 match ip src 0.0.0.0/0 match ip dst 0.0.0.0/0 flowid 20:2

