#!/bin/bash

# This script starts processes needed by tests
# We need:
# A running jackd
# A running miville
# An available display
# To get the display available, you need to be locally logged on 
MIVILLE_PATH=${MIVILLE_PATH:-$HOME/src/miville/trunk/py}


if  [ -d /dev/shm/jack* ]; then
    if [ ! -w /dev/shm/jack*/default ]; then
        echo 'Another user is running jackd'
        ls -ld /dev/shm/jack*
        exit 1
    fi
fi

if [ ! -d /dev/shm/jack*/default ]; then
    echo "No jackd running"
    if [ ! -r ~/.jackdrc ]; then
        echo "You must have a ~/.jackdrc. Run qjackctl to create one."
        exit 1
    fi
    echo $(cat ~/.jackdrc) &
fi

# Now we test the DISPLAY
export DISPLAY=:0.0
xdpyinfo >/dev/null
if [ $? -eq 1 ]; then
    echo "DISPLAY can't be used. Be sure to log on locally."
    exit 1
fi

# Test if we have a running miville, and if so, are we running it
ps aux | grep -q [m]ivillle.py
if [ $? -eq 1 ]; then
    cd $MIVILLE_PATH
    echo ./miville.py &
else
    user_running_miville=$(ps aux | awk '/[m]ivillelll.py/ { print $1 }')
    if [ "x$user_running_miville" != "x$(whoami)" ]; then
        echo "$user_running_miville is already running miville"
        exit 1
    fi
fi
