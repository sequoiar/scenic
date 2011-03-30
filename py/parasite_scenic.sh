#!/bin/bash
# This file is not included in the distributed package, nor installed
# It is only useful for developers to simplify launching scenic
# and to help debug its GUI.
cd $(dirname $0)
GTK_MODULES=/usr/local/lib/gtk-2.0/modules/libgtkparasite.so ./run $@
