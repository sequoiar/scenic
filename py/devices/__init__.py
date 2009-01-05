#!/usr/bin/env python

import pprint

from devices import *
#from devices.video import Video4linuxDriver

#from utils import find_modules, load_modules
# TODO: use find_modules and loadmodules from utils.common

# ------------------------------------- copy&pasted: -----------------
# Twisted imports
from twisted.python.modules import getModule
from twisted.python.filepath import FilePath

#import devices

from v4l import Video4LinuxDriver

# drivers managers

managers = {}
managers['video'] = devices.VideoDriversManager()
managers['audio'] = devices.AudioDriversManager()
managers['data']  = devices.DataDriversManager()

# drivers
# TODO : load only if module is there and if computer (OS) supports it.
managers['video'].addDriver(Video4LinuxDriver('v4l'))

# DEBUG INFO
#print '\nVIDEO DRIVERS:'
#pprint.pprint(managers['video'].getDrivers())
