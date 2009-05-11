#!/bin/bash
# This file must be sourced by bash in your ~/.bashrc
# on the test client machine.
# 
# Make a copy of this file is the test server has an other IP.
# 
# example : 
# source ~/src/miville/trunk/py/test/config_test.sh

export MIVILLE_TEST_REMOTE_HOST="10.10.10.65" 
export MIVILLE_TEST_REMOTE_CONTACT="remote"
export MIVILLE_PATH="$HOME/src/miville/trunk/py"
export PROPULSEART_PATH="$HOME/workspace/miville/trunk/src/main"

alias miville_start="cd $MIVILLE_PATH && python miville.py"

