#!/bin/bash

if [ $# == 0 ]
then
    echo "Error: no gst-module listed"
    exit
fi

if [ $1 == "gst-entrans" ]
then
    svn co https://gentrans.svn.sourceforge.net/svnroot/gentrans gentrans 
else
    cvs -d:pserver:anoncvs@anoncvs.freedesktop.org:/cvs/gstreamer co -D "" $1
fi

