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

Uses v4l2-ctl from Debian/Ubuntu package ivtv-utils
"""
# ----------------------------------------------------------
# System imports
import os, sys, glob
import pprint

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
    name = 'v4l2'
    
    def __init__(self, polling_interval=10.0):
        devices.Driver.__init__(self, polling_interval)
        self.selected_device = None # there can be only one device used
        
    def prepare(self):
        # inherited
        self._populateDevices()
        # we should wait a bit here...
        try:
            tmp = self.devices['/dev/video0']
            self.selected_device = '/dev/video0'
        except KeyError:
            pass
        try:
            tmp = self.find_command('v4l2-ctl') # will fail if not found
        except:
            raise Exception, "v4l2-ctl command not found. Please sudo apt-get install ivtv-utils"
    
    
        
    def on_devices_polling(self):
        """
        Get all infos for all devices
        """
        # backup
        old_devices = self.devices
        
        device_names = glob.glob('/dev/video*')
        self.devices = {}
        if len(device_names) == 0:
            if callback is not None:
                callback(self.devices)
        else:
            for devName in all:
                self.devices[devName] = devices.Device(devName)
                self.devices[devName].setDriver(self)
            self.setSelectedDevice(all[0]) # /dev/video0 TODO : better default device
            self.shell_command_start(['v4l2-ctl','--all'],callback)
        
        # TODO: call the callback only when all the shell
        # calls are done

        #cmd = ['v4l2-ctl','--list-inputs']
        #cmd = ['v4l2-ctl','--get-fmt-video']
        #cmd = ['v4l2-ctl','--get-input']
        #cmd = ['v4l2-ctl','--list-ctrls'] (brightness and contrast)
        
        self.compare_devices()
    
    def _onPopulateDevicesDone(self,callback=None):
        # TODO
        
        self._populate_info[callback] = {} # free shell processes info
        
    
    #def onGetAttributes(self,device):
    #    # inherited
    #    pass
        
    def onDeviceAttributeChange(self,attr,callback=None):
        name = attr.getName()
        command = None
        if name == 'video standard':
            #TODO
            #--set-standard=<num
            #pal-X (X = B/G/H/N/Nc/I/D/K/M/60) or just 'pal'
            #ntsc-X (X = M/J/K) or just 'ntsc'
            #secam-X (X = B/G/H/D/K/L/Lc) or just 'secam'
            standard = attr.getValue() # TODO: get actual value
            command = ['v4l2-ctl','--set-standard='+standard]
            
        elif name == 'width' or name == 'height':
            # TODO
            # --set-fmt-video=width=<w>,height=<h>
            pass
        elif name == 'input':
            # TODO
            #--set-input=0
            pass
        if command is not None:
            #TODO
            pass
    
    def setSelectedDevice(self,device_name):
        """
        Selects the current device name
        """
        self.selected_device = device_name
        #TODO: call shell command
    
    def on_commands_results(self, results, commands, extra_arg=None): 
        for i in range(len(results)):
            result = results[i]
            success, results_infos = result
            if isinstance(results_infos,failure.Failure):
                print "failure ::: ",results_infos.getErrorMessage()  # if there is an error, the programmer should fix it.
            else:
                command = commands[i]
                stdout, stderr, signal_or_code = results_infos
                if success:
                    self._handle_shell_infos_results(command,stdout,callback)
                else:
                    print "failure for command %s" % (command[0])
                    print "stderr: %s" % (stderr)
                    print "signal is ", signal_or_code
        if callback is not None:
            callback(self.getSelectedDevice()) # arg: this driver.
        
    def _handle_shell_infos_results(self,command,results,callback=None):
        if command[0] == 'v4l2-ctl':
            if command[1] == '--all': # reading all attributes
                # method called was listDevices
                device = self.getDevice(self.getSelectedDevice()) # '/dev/video0'
                splitted = results.split('\r\n')
                values = _parse_v4l2_ctl_all(splitted)
                #print "\nSHELL RESULT:"
                #pprint.pprint(splitted)
                #pprint.pprint(values)
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
                    
                    elif key == "video standard":
                        # TODO: v4l2-ctl --?? for better norm/standards names
                        attr = devices.OptionsAttribute(key,values[key],0,['ntsc', 'PAL']) 
                        #TODO : attr.setValue(attr.getIndexForValue( values[key] ),False)
                        device.addAttribute(attr)
        
        log.info('v4l driver: command %s returned.',str(command))
        #print results
    
def _parse_v4l2_ctl_all(lines):
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
                #print "key is "+key+" and value is "+ value
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
