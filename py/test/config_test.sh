#!/bin/bash
# This file must be sourced by bash in your ~/.bashrc
# on the test client machine.
# 
# Make a copy of this file is the test server has an other IP.
# 
# example : 
# source ~/src/miville/trunk/py/test/config_test.sh

export MIVILLE_TEST_REMOTE_HOST="10.10.10.65" # tzing
export MIVILLE_TEST_REMOTE_CONTACT="tzing"
export MIVILLE_PATH="$HOME/src/miville/trunk/py"
export PROPULSEART_PATH="$HOME/src/miville/trunk/src/main"

alias miville_start="cd $MIVILLE_PATH && svn up && python miville.py"

