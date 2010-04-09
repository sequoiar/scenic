#!/bin/bash
# run 
# ln -s gstreamer-0.10-*/common/gst.supp .
G_SLICE=always-malloc G_DEBUG=gc-friendly libtool --mode=execute \ 
valgrind --log-file=vgdump --tool=cachegrind \
--trace-children=yes --num-callers=20 --suppressions=gst.supp -v milhouse $@
