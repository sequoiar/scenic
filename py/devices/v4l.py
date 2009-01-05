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
Manages v4l2. 

TODO: use v4l2-ctl from package ivtv-utils
"""
# ----------------------------------------------------------
# System imports
import os, sys, glob

# Twisted imports
from twisted.internet import reactor, protocol

# App imports
from utils import log
import devices

log = log.start('debug', 1, 0, 'devices')
# ---------------------------------------------------------

class Video4LinuxDriver(devices.VideoDriver):
    """
    Video4linux 2 Driver.
    """
        
    def prepareDriver(self):
        self._populateDevices()
        
    def _populateDevices(self):
        all = glob.glob('/dev/video*')
        self.devices = dict()
        for devName in all:
            self.devices[devName] = devices.Device(devName)
            # TODO: add attributes
            self.devices[devName].setDriver(self)
        
    def listDevices(self,callback):
        self._populateDevices()
        callback(self.devices)
    
    def shell_command_result(self,command,results):
        log.info('v4l driver: command %s returned.',str(command))
        
    def _v4l_command(self,deviceObj,command):
        """
        v4lctl -c /dev/videoN [command]
        """
        devName = '/dev/video0' # TODO
        command.insert(0,devName)
        command.insert(0,'-c')
        command.insert(0,'v4lctl')
        self.shell_command_start(command)

    def v4l_set_norm(self,deviceObj,norm='NTSC'):
        """
        Sets TV norm to PAL, NTSC or SECAM
        """
        self._v4l_command(deviceObj,['setnorm',norm])
    
    def v4l_input_next(self,deviceObj):
        """
        Tries next input. (composite0,composite1,Television...
        """
        self._v4l_command(deviceObj,['setinput','next'])
    
    def v4l_set_attr(self,deviceObj,attr,value):
        """
        Wrapper for setting an attribute using v4lctl
        """
        self._v4l_command(deviceObj,['setattr',attr,value])

