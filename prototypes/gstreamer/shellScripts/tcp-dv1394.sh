#!/bin/bash

gst-launch dv1394src ! tcpclientsink host=192.168.1.217 port=10010
