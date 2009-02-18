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
VideoDriver that manages v4l2 devices.

Uses v4l2-ctl from Debian/Ubuntu package ivtv-utils
"""
#TODO:: will notify with an error in case some command-line tool is not found. 
# ----------------------------------------------------------
# System imports
import os
import sys
import glob
import pprint

# Twisted imports
from twisted.internet import reactor, protocol
from twisted.python import procutils
from twisted.python import failure

# App imports
from utils import log as logger
from utils.commands import *
import devices
#from devices import *
log = logger.start('debug', 1, 0, 'devices_v4l2')

# ---------------------------------------------------------
def _parse_v4l2_ctl_all(lines):
    """
    Parses the output of the `v4l2-ctl --all -d /dev/video0` command
    
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
                norm = line.strip('\t').strip('\n')
                if norm.startswith('NTSC-'):
                    norm = 'ntsc'
                elif norm.startswith('PAL-'):
                    norm = 'pal'
                elif norm.startswith('SECAM-'):
                    norm = 'secam'
                results['norm'] = norm
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
    Parses the output of `v4l2-ctl --list-inputs -d /dev/video0`
    
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
    
    self.selected_device = "/dev/video0"
    """
    name = 'v4l2'
    
    def __init__(self, polling_interval=15.0):
        devices.Driver.__init__(self, polling_interval)
        self.selected_device = '/dev/video0' # Use this to set the video card we use.
        
    def prepare(self):
        """
        Returns a Deferred instance? no
        """
        try:
            tmp = find_command('v4l2-ctl') # will fail if not found
        except:
            self.api.notify(None, "v4l2-ctl command not found. Please sudo apt-get install ivtv-utils", "error")
            #raise CommandNotFoundError("v4l2-ctl command not found. Please sudo apt-get install ivtv-utils")
            log.error("v4l2-ctl command not found. Please sudo apt-get install ivtv-utils")
        return devices.Driver.prepare(self)
    
    def _on_devices_polling(self, caller=None, event_key=None):
        """
        Get all infos for all devices.
        Starts the commands.
        
        Must return a Deferred instance.
        """
        # TODO: we must populate an other dict 
        # so that we can still access the data while populating.
        # since it is asynchronous
        device_names = glob.glob('/dev/video*')
        commands = []
        extra_arg = []

        for name in device_names:
            device = devices.Device(name)
            self._add_new_device(device) # in_new_devices # TODO : make add_device cleaner.
            # TODO: does it notify the api ?
            
            # add its attributes with default values.
            # next, we will populate the rael options
            attr = devices.OptionsAttribute('input', 'Composite0', 'Composite0', ['Composite0', 'Composite1', 'S-Video']) # 'Composite2'
            device.add_attribute(attr)
            
            # now, the commands (TODO)
            commands.append(['v4l2-ctl', '--all', '-d', name])
            commands.append(['v4l2-ctl', '--list-inputs', '-d', name])
            for i in range(2):
                extra_arg.append(name) # twice
            # TODO:
            #cmd = ['v4l2-ctl','--list-inputs']
            #cmd = ['v4l2-ctl','--get-fmt-video']
            #cmd = ['v4l2-ctl','--get-input']
            #cmd = ['v4l2-ctl','--list-ctrls'] (brightness and contrast)
        #print "commands_start(%s, %s, %s)" % (str(commands), str(self.on_commands_results), extra_arg)
        return commands_start(commands, self.on_commands_results, extra_arg, caller) # returns a Deferred
        # TODO: adds an other arg to commands_start for the caller object
    
    def on_attribute_change(self, attr, caller=None, event_key=None):
        name = attr.name
        val = attr.get_value() # new value
        dev_name = attr.device.name
        ret = None
        
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
            ret = single_command_start(command, self.on_commands_results, 'attr_change', caller)             
        elif name == 'width' or name == 'height': 
            # --set-fmt-video=width=<w>,height=<h>
            # TODO: check if within range. fix if not
            # if both attributes are changes at the same time before the next poll, 
            # the second one is going to set the first back back to its prev value.
            # NO ! it's ok, since we retrieve the value from what the programmer had set,
            # and not the actual value of the device.
            if name == 'width':
                width = val
                height = attr.device.attributes['height'].get_value()
            else:
                height = val
                width = attr.device.attributes['width'].get_value()
            command = ['v4l2-ctl', '--set-fmt-video=width=%d,height=%d' % (width, height), '-d', dev_name]
            ret = single_command_start(command, self.on_commands_results, 'attr_change', caller) 
        
        elif name == 'input': # --set-input=<num>
            # TODO parse data to get possible values
            # in order to puts them in an OptionsAttribute
            #inputs =  # list
            #inputs = ['Composite0', 'Composite1', 'S-Video'] # 'Composite2', 
            if val in attr.options:
                index = attr.options.index(val)
                command = ['v4l2-ctl', '--set-input=' + index, '-d', dev_name]
                ret = single_command_start(command, self.on_commands_results, 'attr_change', caller) 
 
        if command is not None:
            #TODO
            pass
        return ret # Deferred
    
    def on_commands_results(self, results, commands, extra_arg=None, caller=None): 
        """
        Parses commands results
        
        extra_arg can be device name, such as /dev/video0
        """
        for i in range(len(results)):
            result = results[i]
            success, results_infos = result
            if isinstance(results_infos, failure.Failure):
                msg = "command failure :" + results_infos.getErrorMessage()  # if there is an error, the programmer should fix it.
                log.error(msg)
            else:
                command = commands[i]
                stdout, stderr, signal_or_code = results_infos
                if success:
                    arg = None
                    if isinstance(extra_arg, list):
                        arg = extra_arg[i]
                    else:
                        arg = extra_arg
                    self._handle_shell_infos_results(command, stdout, arg, caller) # this is where all the poutine happens
                else:
                    msg = "non-succes for command %s" % (command[0])
                    msg += " stderr: %s" % (stderr)
                    msg += " signal is " + str(signal_or_code)
                    log.error(msg)
        if extra_arg == 'attr_change':
            event_name = 'attr'
        else:
            self._on_done_devices_polling(caller) # attr_change, 
    
    def _handle_shell_infos_results(self, command, results, extra_arg=None, caller=None):
    	"""
        Handles results for only one command received by on_commands_results
        
        See on_command_results() 
        """
        device_name = extra_arg
        #print 'v4l','_handle_shell_infos_results'
        event_type = ''
        if extra_arg == 'attr_change':
            event_type = 'attr_change'
        if command[0] == 'v4l2-ctl':
            if command[1] == '--all': # reading all attributes
                try:
                    device = self._new_devices[device_name] # '/dev/video0'
                except KeyError:
                    log.info('no device ' + str(extra_arg))
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
                            #attr = devices.OptionsAttribute(key, , 0, ['Composite0', 'Composite1', 'S-Video']) # 'Composite2'
                            #device.add_attribute(attr)
                            device.attributes[key].set_value(value, False)
                        elif key == "norm":
                            attr = devices.OptionsAttribute(key, dic[key], 'ntsc', ['ntsc', 'pal', 'secam'])
                            device.add_attribute(attr) # we could check if value is valid
            elif command[1] == '--list-inputs':
                try:
                    device = self._new_devices[extra_arg] #'/dev/video0']
                except KeyError:
                    log.info('no device ' + str(extra_arg)) #/dev/video0
                else:
                    lines = results.splitlines()
                    inputs = _parse_v4l2_ctl_list_inputs(lines)
                    device.attributes['input'].options = lines
                    
        #log.info('v4l driver: command %s returned.', str(command))
        #print results
        
        if event_type == 'attr_change':
            self.poll_now(caller, 'device_modify_attribute')
            argument = True # TODO: check if change is successful
            self._call_event_listener('device_modify_attribute', argument, caller)
            
#devices.managers['video'].add_driver(Video4LinuxDriver()) # only one instance.
# TODO : load only if module is there and if computer (OS) supports it.

def start(api):
    """
    Starts this driver
    """
    driver = Video4LinuxDriver()
    driver.api = api
    devices.managers['video'].add_driver(driver)
    reactor.callLater(0, driver.prepare)


    
    
