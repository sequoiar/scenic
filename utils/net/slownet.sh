#!/bin/bash
# run this script as root to create 150ms delay conditions between this host's ip and DESTHOST, defaults
# to 150 000 usecs
#
usage ()
{
    echo "Usage: sudo $(basename $0) <INTERFACE> <DEST_HOST> [DELAY_USEC]"
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

if [ $# -lt 2 ]
then
    echo "Invalid arguments."
    usage
    exit
fi

INTERFACE=$1
DEST_HOST=$2
DELAY_USEC=${3:-150000}
SOURCE_HOST=$( ifconfig ${INTERFACE} | awk '/inet addr/ { sub(/addr:/,"",$2);print $2}' )

if [ $DELAY_USEC -lt 0 ]
then
    echo "Died trying to simulate negative latency, apparently time travel is still impossible."
    exit 1
fi

echo "Delaying connection by ${DELAY_USEC} useconds between src ${SOURCE_HOST} and dest ${DEST_HOST} on interface ${INTERFACE}"

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

