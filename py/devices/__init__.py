#!/usr/bin/env python

from devices import *
#from devices.video import Video4linuxDriver

#from utils import find_modules, load_modules
# TODO: use find_modules and loadmodules from utils.common

# ------------------------------------- copy&pasted: -----------------
# Twisted imports
from twisted.python.modules import getModule
from twisted.python.filepath import FilePath

import devices

from  v4l import Video4LinuxDriver
