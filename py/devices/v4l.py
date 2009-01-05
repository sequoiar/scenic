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
from twisted.python import procutils
# App imports
from utils import log
import devices

log = log.start('debug', 1, 0, 'devices')
# ---------------------------------------------------------

class Video4LinuxDriver(devices.VideoDriver):
    """
    Video4linux 2 Driver.
    """
    def __init__(self, name):
        devices.Driver.__init__(self, name)
        self.selected_device = None # there can be only one device used
    
    def prepareDriver(self):
        # inherited
        self._populateDevices()
        try:
            tmp = self.devices['/dev/video0']
            self.selected_device = '/dev/video0'
        except KeyError:
            pass
        
        try:
            executable = procutils.which('v4l2-ctl')[0]
        except IndexError:
            raise devices.CommandNotFoundException, "v4l2-ctl command not found. Please sudo apt-get install ivtv-utils"
        
    def _populateDevices(self,callback=None):
        all = glob.glob('/dev/video*')
        self.devices = dict()
        if len(all) == 0:
            if callback is not None:
                callback(self.devices)
        else:
            for devName in all:
                self.devices[devName] = devices.Device(devName)
                self.devices[devName].setDriver(self)
            self.setSelectedDevice(all[0]) # TODO : better default device
            self.shell_command_start(['v4l2-ctl','--all'],callback)
    
    def onGetAttributes(self,device):
        # inherited
        pass
    
    def onDeviceAttributeChange(self,attribute):
        pass
        
    def setSelectedDevice(self,device_name):
        """
        Selects the current device name
        """
        self.selected_device = device_name
    
    def getSelectedDevice(self):
        return self.selected_device
    
    def listDevices(self,callback):
        # inherited
        self._populateDevices(callback)
        
    
    def shell_command_result(self,command,results,callback=None):
        # inherited
        if command[0] == 'v4l2-ctl':
            if command[1] == '--all': # reading all attributes
                # method called was listDevices
                device = self.getDevice(self.getSelectedDevice())
                values = parse_v4l2_ctl(results.split())
                for key in values:
                    if key in ['driver','card','pixel format']:
                        device.addAttribute(devices.StringAttribute(key,values[key],'no default'))
                    elif key == 'width':
                        device.addAttribute(devices.IntAttribute(key,int(values[key]),640,320,9999)) # TODO better min/max
                    elif key == 'height':
                        device.addAttribute(devices.IntAttribute(key,int(values[key]),480,240,9999)) # TODO better min/max 
                    elif key == 'input':
                        # Composite0, Composite1,  Composite2, S-Video
                        # TODO: v4l2-ctl --list-inputs
                        attr = devices.OptionsAttribute(key,values[key],0,['Composite0', 'Composite1', 'Composite2', 'S-Video'])
                        attr.setValue(attr.getIndexForValue(values[key]),False)
                        device.addAttribute(attr)
                    elif category == "Video Standard":
                        # TODO: v4l2-ctl --?? for better norm/standards names
                        attr = devices.OptionsAttribute(key,values[key],0,['ntsc', 'PAL']) 
                        attr.setValue(attr.getIndexForValue(values[key]),False)
                        device.addAttribute(attr)
                if callback is not None:
                    callback(self.devices)
                
        log.info('v4l driver: command %s returned.',str(command))


class delete_me:
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


def parse_v4l2_ctl(lines):
    """
    Parses the output of the "v4l2-ctl --all" shell command
    
    Returns a dict.
    """
    results = dict()
    category = None
    sub_category = None
    pixel_format_is_done = False
    
    for line in lines:
        is_value = False
        if line.startswith('\t\t'):
            pass # subvalue
        elif line.startswith('\t'):
            # value ------------------------------
            splitted = line.strip('\t').split(':')
            try:
                key = splitted[0].strip()
                value = splitted[1].strip()
                is_value = True
                print "key is "+key+" and value is "+ value
            except IndexError:
                pass
            if is_value:
                if key == 'Driver name':
                    results['driver'] = value
                elif key == 'card type':
                    results['card'] = value
                elif key == 'Pixel Format' and not pixel_format_is_done:
                    results['pixel format'] = value
                    pixel_format_is_done= True
                elif key == 'Width/Height':
                    dimen = value.split('/')
                    results['width'] = dimen[0]
                    results['height'] = dimen[1]
                elif key == 'Video input':
                    results['input'] = value.split('(')[1].split(')')[0] # 0 (Composite0)
                    # TODO : possibilities are Composite0, Composite1, 
                # norm
            elif category == "Video Standard":
                results['video standard'] = line.strip('\t').strip('\n')
            
        elif line.find(":") > 0:
            try:
                category = line.split(':')[0]
            except IndexError:
                pass
        elif line.find("Video Standard") == 0:
            category  = "Video Standard"
    
    return results
