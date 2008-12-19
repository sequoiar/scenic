# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.

"""
Manages v4l2
"""

# System imports
import os, sys, glob

# Twisted imports
from twisted.internet import reactor, protocol

# App imports
from utils import log
import devices

log = log.start('debug', 1, 0, 'devices')

class Video4LinuxDriver(devices.Driver):
    """
    Video4linux 2 Driver.
    """
    def start(self):
        pass
    def list(self):
        return glob.glob('/dev/video*')
    def get(self):
        return None
    def shell_command_result(self,command,results):
        pass #print "results from command %s are :%s" % (command[0], results)
    
