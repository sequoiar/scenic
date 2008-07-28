#!/bin/sh

chuck --silent --srate48000 frameImpulse.ck
./mkWhiteFrame.m
./muxAv.sh

