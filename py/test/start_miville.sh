#!/bin/bash
# Licensed under GPLv2 or later

# This script starts processes needed by tests
# We need:
# A running jackd
# A running miville
# An available display
# To get the display available, you need to be locally logged on 

# Get MIVILLE_PATH or assign a default value
MIVILLE_PATH=${MIVILLE_PATH:-$HOME/src/miville/trunk/py}

if  [ -d /dev/shm/jack-* ]; then
    if [ ! -w /dev/shm/jack-* ]; then
        echo 'User $(stat -c %U /dev/shm/jack*) is running jackd'
        exit 1
    fi
fi

if [ ! -d /dev/shm/jack-* ]; then
    echo "No jackd running"
    if [ ! -r ~/.jackdrc ]; then
        echo "You must have a ~/.jackdrc. Run qjackctl to create one."
        exit 1
    fi
    $(cat ~/.jackdrc) &
fi

# Now we test the DISPLAY
export DISPLAY=:0.0
xdpyinfo >/dev/null
if [ $? -eq 1 ]; then
    echo "DISPLAY $DISPLAY can't be used. Be sure to log on locally."
    exit 1
fi

# Test if we have a running miville, and if so, are we running it
ps aux | grep -q [m]ivillle.py
if [ $? -eq 1 ]; then
    cd $MIVILLE_PATH
    HOME=/var/tmp
    rm -f /var/tmp/.sropulpof/.spropulpof.adb
    rm -rf /var/tmp/.sropulpof
    ./miville.py
else
    user_running_miville=$(ps aux | awk '/[m]iville.py/ { print $1 }')
    if [ "x$user_running_miville" != "x$(whoami)" ]; then
        echo "$user_running_miville is already running miville"
        exit 1
    fi
fi
