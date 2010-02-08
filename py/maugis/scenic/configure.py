#!/usr/bin/env python
# -*- coding: utf-8 -*-

APPNAME = "scenic"
VERSION = "0.3.6"

import os
from scenic import data # TODO: change package data to configure.ac variable
# contains the glade, gtkrc and locale files.
PKGDATADIR = os.path.dirname(data.__file__) #'/usr/local/share/scenic'
# not used yet. Will contain custom icons. 
PIXMAPDIR = os.path.dirname(data.__file__) #'/usr/local/share/pixmaps'
