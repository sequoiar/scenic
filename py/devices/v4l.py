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
VideoDriver (instance) that manages v4l2 devices.

Uses v4l2-ctl from Debian/Ubuntu package ivtv-utils
"""
# ----------------------------------------------------------
# System imports
import os, sys, glob
import pprint

# Twisted imports
from twisted.internet import reactor, protocol
from twisted.python import procutils
from twisted.python import failure
# App imports
from utils import log
import devices

log = log.start('debug', 1, 0, 'devices')
# ---------------------------------------------------------
def _parse_v4l2_ctl_all(lines):
    """
    Parses the output of the "v4l2-ctl --all" command
    
    Returns a dict.
    """
    results = dict()
    category = None
    sub_category = None
    pixel_format_is_done = False
    #print lines
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
                results['norm'] = line.strip('\t').strip('\n')
        elif line.find(":") > 0:
            try:
                category = line.split(':')[0]
            except IndexError:
                pass
        elif line.find("Video Standard") == 0:
            category  = "Video Standard"
    #pprint.pprint(results)
    return results

def _parse_v4l2_ctl_list_inputs(lines):
    """
    Parses the output of v4l2-ctl --list-inputs
    
    Returns a list with all input names.
    Their index is their number.
    """
    inputs = []
    for line in lines:
        if line.startswith('\t'):
            splitted = line.strip('\t').split(':')
            try:
                key = splitted[0].strip()
                value = splitted[1].strip()
            except IndexError:
                pass
            else:
                if key == 'Name':
                    inputs.append(value)
    return inputs
# ---------------------------------------------------------

class Video4LinuxDriver(devices.VideoDriver):
    """
    Video4linux 2 Driver.
    """
    name = 'v4l2'
    
    def __init__(self, polling_interval=15.0):
        devices.Driver.__init__(self, polling_interval)
        
    def prepare(self):
        try:
            tmp = self.find_command('v4l2-ctl') # will fail if not found
        except:
            raise Exception("v4l2-ctl command not found. Please sudo apt-get install ivtv-utils")
        devices.Driver.prepare(self)
    
    def _on_devices_polling(self):
        """
        Get all infos for all devices.
        Starts the commands.
        """
        # TODO: we must populate an other dict 
        # so that we can still access the data while populating.
        # since it is asynchronous
        device_names = glob.glob('/dev/video*')
        for name in device_names:
            device = devices.Device(name)
            self.add_device(device, True) # in_new_devices # TODO : make add_device cleaner.
            # TODO: does it notify the api ?
            
            # add its attributes with default values.
            attr = devices.OptionsAttribute('input', 'Composite0', 'Composite0', ['Composite0', 'Composite1', 'S-Video']) # 'Composite2'
            device.add_attribute(attr)
            
            # now, the commands (TODO)
            commands = [
                ['v4l2-ctl', '--all', '-d', name],
                ['v4l2-ctl', '--list-inputs', '-d', name]
                ]
            self.commands_start(commands, name)
        #cmd = ['v4l2-ctl','--list-inputs']
        #cmd = ['v4l2-ctl','--get-fmt-video']
        #cmd = ['v4l2-ctl','--get-input']
        #cmd = ['v4l2-ctl','--list-ctrls'] (brightness and contrast)
        
    def on_attribute_change(self, attr):
        name = attr.name
        val = attr.get_value()
        dev_name = attr.device.name
        
        #print ">>>>>>>>>>val", val, type(val)
        if name == 'norm': # 'ntsc', 'secam' or 'pal'
            # --set-standard=<num
            # pal-X (X = B/G/H/N/Nc/I/D/K/M/60) or just 'pal'
            # ntsc-X (X = M/J/K) or just 'ntsc'
            # secam-X (X = B/G/H/D/K/L/Lc) or just 'secam'
            norm = 'ntsc' # default
            
            if val.lower().startswith('pal'):
                norm = 'pal'
            elif val.lower().startswith('secam'):
                norm = 'secam'
            command = ['v4l2-ctl', '--set-standard=' + norm, '-d', dev_name]
            self.single_command_start(command)
            
        elif name == 'width' or name == 'height': 
            # --set-fmt-video=width=<w>,height=<h>
            # TODO: check if within range. fix if not
            if name == 'width':
                width = val
                height = attr.device.attributes['height'].get_value()
            else:
                height = val
                width = attr.device.attributes['width'].get_value()
            command = ['v4l2-ctl', '--set-fmt-video=width=%d,height=%d' % (width, height), '-d', dev_name]
            self.single_command_start(command)
        
        elif name == 'input': # --set-input=<num>
            # TODO parse data to get possible values
            # in order to puts them in an OptionsAttribute
            #inputs =  # list
            #inputs = ['Composite0', 'Composite1', 'S-Video'] # 'Composite2', 
            if val in attr.options:
                index = attr.options.index(val)
                command = ['v4l2-ctl', '--set-input=' + index, '-d', dev_name]
                self.single_command_start(command)
                
        if command is not None:
            #TODO
            pass
    
    def on_commands_results(self, results, commands, extra_arg=None): 
        """
        Parses commands results
        
        extra_arg can be device name, such as /dev/video0
        """
        for i in range(len(results)):
            result = results[i]
            success, results_infos = result
            if isinstance(results_infos, failure.Failure):
                print "failure ::: ", results_infos.getErrorMessage()  # if there is an error, the programmer should fix it.
            else:
                command = commands[i]
                stdout, stderr, signal_or_code = results_infos
                if success:
                    self._handle_shell_infos_results(command, stdout, extra_arg)
                else:
                    print "failure for command %s" % (command[0])
                    print "stderr: %s" % (stderr)
                    print "signal is ", signal_or_code
        
    
    def _handle_shell_infos_results(self, command, results, extra_arg=None):
    	#print 'v4l','_handle_shell_infos_results'
        if command[0] == 'v4l2-ctl':
            if command[1] == '--all': # reading all attributes
                try:
                    device = self.new_devices[extra_arg] # '/dev/video0'
                except KeyError:
                    log.info('no device '+str(extra_arg))
                else:
               	    splitted = results.splitlines()
                    dic = _parse_v4l2_ctl_all(splitted)
                    #print "\nSHELL RESULT:"
                    #pprint.pprint(splitted)
                    #pprint.pprint(values)
                    #print dic
                    for key in dic:
                        value = dic[key]
                        if key in ['driver','card','pixel format']:
                            device.add_attribute(devices.StringAttribute(key, dic[key], 'no default'))
                        elif key == 'width':
                            device.add_attribute(devices.IntAttribute(key, int(dic[key]), 640, 320, 9999)) # TODO better min/max
                        elif key == 'height':
                            device.add_attribute(devices.IntAttribute(key, int(dic[key]), 480, 240, 9999)) # TODO better min/max 
                        elif key == 'input':
                            # Composite0, Composite1,  Composite2, S-Video
                            # TODO: v4l2-ctl --list-inputs
                            #attr = devices.OptionsAttribute(key, , 0, ['Composite0', 'Composite1', 'S-Video']) # 'Composite2'
                            #device.add_attribute(attr)
                            device.attributes[key].set_value(value, False)
                        elif key == "norm":
                            # TODO: v4l2-ctl --?? for better norm/standards names
                            attr = devices.OptionsAttribute(key, dic[key], 'NTSC', ['NTSC', 'PAL']) 
                            #TODO : attr.setValue(attr.getIndexForValue( values[key] ),False)
                            device.add_attribute(attr)
            elif command[1] == '--list-inputs':
                try:
                    device = self.new_devices[extra_arg] #'/dev/video0']
                    # TODO: check if there are many devices, how the v4l2-ctl command works.
                except KeyError:
                    log.info('no device '+str(extra_arg)) #/dev/video0
                else:
                    lines = results.splitlines()
                    inputs = _parse_v4l2_ctl_list_inputs(lines)
                    device.attributes['input'].options = lines
                    
        log.info('v4l driver: command %s returned.', str(command))
        #print results
        self._on_done_devices_polling()
        

devices.get_manager('video').add_driver(Video4LinuxDriver()) # only one instance.
# TODO : load only if module is there and if computer (OS) supports it.

def start(api):
    """TODO"""
    pass
    
