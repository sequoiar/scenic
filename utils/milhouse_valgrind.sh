#!/bin/bash
G_SLICE=always-malloc G_DEBUG=gc-friendly libtool --mode=execute valgrind --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=20 --log-file=vgdump milhouse $@
