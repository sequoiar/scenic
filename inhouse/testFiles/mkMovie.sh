#!/bin/sh

chuck --silent frameImpulse.ck
./mkWhiteFrame.m
./muxAv.sh

