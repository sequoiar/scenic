#!/bin/bash

cd $1
cvs update -dC $1
./autogen.sh
make && sudo make install

