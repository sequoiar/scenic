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
import sys
import traceback

# Twisted imports
from twisted.internet import reactor
from twisted.internet.error import CannotListenError

# App imports
from miville import api
#import streams
from miville.utils import common
from miville.utils import log
from miville.utils.observer import Subject
from miville import addressbook
#from miville import settings
from miville.protocols import com_chan
from miville import connectors
from miville import devices

# module variables
core = None

log = log.start('debug', True, True, 'core') # LOG TO FILE = True

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
        self.addressbook = addressbook.AddressBook(self.config.addressbook_filename, self.api)
        #self.services = self.find_services()
        
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
                raise
            except Exception, e:
                log.error('Unable to start UI module %s. %s %s' % (mod.__name__, e, sys.exc_info())) # traceback please
                traceback.print_exc()
                raise

#     def find_services(self):
#         """
#         Find all the different audio/video/data services
#         """
#         services = {}
#         for kind in ('audio', 'video'):
#             mods = getModule('streams.' + kind).iterModules()
#             for service in mods:
#                 services[service.name] = service
#         return services

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
        if core.addressbook != None:
            core.addressbook.write(False)
    except AttributeError:
        pass
    devices.stop()
    # that'll do something, but maybe not what you expect.
    reactor.disconnectAll()
    reactor.removeAll()
    #reactor.stop()
    #del reactor
    sys.exit(app_return_val)

