#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.

"""
The core of miville loads most of its modules and start some servers.

See also the api.py for more modules init and servers startup.

Contains the core configuration of miville.
"""
import os
import sys
import traceback
import socket

# Twisted imports
from twisted.internet import reactor, task
from twisted.python.modules import getModule
from twisted.internet.error import CannotListenError

# App imports
from miville import ui
from miville import api
#import streams
from miville.utils import log, Subject, common
from miville import addressbook
from miville import settings
from miville.protocols import com_chan
from miville import connectors
from miville import devices

# module variables
core = None

class MivilleConfiguration(object):
    """
    Stores configuration options for miville. 
    
    Default values are set here.
    They can be overriden using a dict passed to the constructor.
    They can also be changed manually, simply. 
    This class is not actually used yet.
    """
    def __init__(self, dictionary=None):
        self.verbose = False # useless so far
        # network
        self.com_chan_port = 31054
        self.telnet_port = 14444
        self.restart_jackd = False
        self.connector_port = 2222
        self.web_port = 8080
        self.iperf_port = 5001 # iperf's default
        #self.midi_port = 44000
        # self.ipcp_port = 999999999999
        self.port_numbers_offset = 0
        self.listen_to_interfaces = '' # means all interfaces
        self.ui_network_interfaces = ['127.0.0.1'] # default is only local host
        self.enable_escape_sequences = True
        # files
        self.miville_home = os.path.expanduser("~/.miville") # TODO: change for ~/.miville
        self.addressbook_filename = 'addressbook.txt' # TODO: "contacts.txt"
        self.settings_presets_filename = "presets.txt" # TODO: presets.txt
        self.settings_filename = "settings.txt" # TODO: settings.txt
        
        # update the attributes to match passed dict
        if dictionary is not None:
            self.update_dict(dictionary)

    def update_dict(self, dico):
        self.__dict__.update(dico)

    def print_values(self):
        for key, value in self.__dict__.items():
            print "%20s: %s" % (key, value)


class Core(Subject):
    """
    Main class of the application and containing the 'Model' in the MVC
    """    
    def __init__(self, config_object):
        """
        defines attributes and does the startup routine.

        :arg config: MivilleConfiguration instance.
        """
        Subject.__init__(self)
        self.config = config_object
        common.MIVILLE_HOME = self.config.miville_home
        self.uis = None
        self.com_chan_port = self.config.com_chan_port + self.config.port_numbers_offset 
        self.api = api.ControllerApi(self.notify)
        self.api.core = self 
        # much important is to start the devices modules
        devices.start(self.api) # passing this api as an argument, 
                                # so that both share the same notify method.
        self.load_uis()
        # TODO: rename this addressbook !
        self.adb = addressbook.AddressBook(self.config.addressbook_filename, self.api)
        #self.engines = self.find_engines()
        # create the settings collection
        settings.PRESETS_FILENAME = self.config.settings_presets_filename
        settings.SETTINGS_FILENAME = self.config.settings_filename
        self.settings = settings.Settings()
        #self.curr_setting = self.settings.select()
        # TODO: causes a couldn't listen error if another miville runs on the same port. 
        # and makes the application crash. 
        # maybe something more elegant could be done.
        self.connectors = connectors.load_connectors(self.api, self.config.connector_port + self.config.port_numbers_offset, self.config.listen_to_interfaces)
        com_chan.start(self.api, connectors.connections, self.com_chan_port, self.config.listen_to_interfaces)
        self.api._start(self)
        if self.config.verbose:
            print 'Current Miville configuration: '
            for k in sorted(self.config.__dict__):
                v = self.config.__dict__[k]
                print "        %30s  :  %s" % (k, v)

        print "Miville is ready." # This is the only "print" allowed in the Miville application
        
    def load_uis(self):
        """
        Loads the user interface modules
        """
        self.uis = common.load_modules(common.find_modules('ui'))
        count = 0
        for mod in self.uis:
            # interfaces = self.config.listen_to_interfaces
            interfaces = self.config.ui_network_interfaces
            if mod.__name__.find('cli') != -1:
                port = self.config.telnet_port + self.config.port_numbers_offset
                mod.enable_escape_sequences = self.config.enable_escape_sequences 
            elif mod.__name__.find('web') != 1:
                port = self.config.web_port + self.config.port_numbers_offset
            else: 
                log.error('unknown user interface')
            try:
                mod.start(self, port, interfaces)
            except CannotListenError, e:
                log.error('Unable to start UI module %s. %s %s' % (mod.__name__, e, sys.exc_info())) # traceback please
                log.error("Port unavailable. There is probably an other miville running on this machine. Try with -o option.")
                log.error("Exiting.")
                exit(1) # ends the program
            except Exception, e:
                log.error('Unable to start UI module %s. %s %s' % (mod.__name__, e, sys.exc_info())) # traceback please
                traceback.print_exc()

#     def find_engines(self):
#         """
#         Find all the different audio/video/data engines
#         """
#         engines = {}
#         for kind in ('audio', 'video'):
#             mods = getModule('streams.' + kind).iterModules()
#             for engine in mods:
#                 engines[engine.name] = engine
#         return engines



def main(config_object):    
    """
    Startup of the application.
    """
    global core
    
    core = Core(config_object)
    
#    l = task.LoopingCall(chk_ob, core)
#    l.start(2.0, False)

def exit(app_return_val=0):
    """
    Called on application exit
    
    Cancel delayed calls, stop ports, disconnect connections. 
    """
    try:
        if core.adb != None:
            core.adb.write(False)
    except AttributeError:
        pass
    devices.stop()
    # that'll do something, but maybe not what you expect.
    reactor.disconnectAll()
    reactor.removeAll()
    #reactor.stop()
    #del reactor
    sys.exit(app_return_val)

