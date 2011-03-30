#!/bin/bash
#
# run this script as root to create packet loss conditions (defaults to 1%)
# between this host's ip and DEST IP
#

usage () {
    echo "Usage: sudo $(basename $0) <INTERFACE> <DEST_HOST> [PACKET_LOSS]"
}

if [ $# -lt 1 ]
then
    echo "Invalid arguments."
    usage
    exit 1
fi

if [ $1 = "--help" ]
then
    usage
    exit
fi

if [ $# -lt 2 ]
then
    echo "Invalid arguments."
    usage
    exit 1
fi

INTERFACE=$1
DEST_HOST=$2
PACKET_LOSS=${3:-1}
SOURCE_HOST=$( ifconfig ${INTERFACE} | awk '/inet addr/ { sub(/addr:/,"",$2);print $2}' )

if [ $PACKET_LOSS -gt 10 ]
then
    echo "Come on\! More than 10%?"
    exit 1
elif [ $PACKET_LOSS -lt 0 ]
then
    echo "Come on\! Negative packet loss?"
    exit 1
fi


echo "Trashing connection between src ${SOURCE_HOST} and dest ${DEST_HOST} on interface ${INTERFACE}"

# clean up
tc qdisc del dev ${INTERFACE} root
# root node
tc qdisc add dev ${INTERFACE} root handle 1: prio bands 10
# netem qdisc
tc qdisc add dev ${INTERFACE} parent 1:1 handle 10: netem loss ${PACKET_LOSS}%
tc filter add dev ${INTERFACE} protocol ip parent 1:0 prio 1 u32 match ip src ${SOURCE_HOST}/32 match ip dst ${DEST_HOST}/32 flowid 10:1
# other qdisc
tc qdisc add dev ${INTERFACE} parent 1:2 handle 20: pfifo
tc filter add dev ${INTERFACE} protocol ip parent 1:0 prio 2 u32 match ip src 0.0.0.0/0 match ip dst 0.0.0.0/0 flowid 20:2

