#!/usr/bin/env python
# -*- coding: utf-8 -*-

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

import sys

# Twisted imports
from twisted.internet import reactor, task
from twisted.python.modules import getModule

# App imports
import ui
import api
import streams
from utils import log, Subject, common
import addressbook
import settings
from protocols import com_chan
import connectors
import devices
import socket

from twisted.internet.error import CannotListenError

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
        self.com_chan_port = 37054
        self.addressbook_filename = "contacts.txt"
        self.port_numbers_offset = 0
        self.verbose = False

        # update the attributes to match passed dict
        if dictionary is not None:
            self.__dict__.update(dictionary)

    def update_dict(self, dico):
        self.__dict__.update(dictionary)


    def print_values(self):
        for key, value in self.__dict__.items():
            print "%20s: %s" % (key, value)


class Core(Subject):
    """
    Main class of the application and containing the 'Model' in the MVC
    """    
    def __init__(self):
        """
        defines attributes and does the startup routine.

        If Miville is started by any argument on the CLI, its basic connector will listen on port 
        37055 instead of the default 37054. Useful for debugging.
        """
        Subject.__init__(self)
        self.config = MivilleConfiguration()
        self.uis = None
        self.com_chan_port = 37054
        self.api = api.ControllerApi(self.notify)
        
        # much important is to start the devices modules
        devices.start(self.api) # passing this api as an argument, 
                                # so that both share the same notify method.
        self.load_uis()
        # TODO: rename this addressbook !
        self.adb = addressbook.AddressBook('sropulpof.adb', self.api)
        self.engines = self.find_engines()
        # create the settings collection
        self.settings = settings.Settings()
        #self.curr_setting = self.settings.select()
        # TODO: causes a couldn't listen error if another miville runs on the same port. 
        # and makes the application crash. 
        # maybe something more elegant could be done.
        self.connectors = connectors.load_connectors(self.api)
        if len(sys.argv) > 1:
            self.com_chan_port += 1
        com_chan.start(connectors.connections, self.com_chan_port)
        self.api._start(self)
        
    def load_uis(self):
        """
        Loads the user interface modules
        """
        self.uis = common.load_modules(common.find_modules('ui'))
        count = 0
        for mod in self.uis:
            try:
                if len(sys.argv) > 1:
                    mod.start(self, int(sys.argv[1]) + count)
                    count += 10
                else:
                    mod.start(self)
            except:
                log.error('Unable to start UI module %s.' % mod.__name__)

    def find_engines(self):
        """
        Find all the different audio/video/data engines
        """
        engines = {}
        for kind in ('audio', 'video'):
            mods = getModule('streams.' + kind).iterModules()
            for engine in mods:
                engines[engine.name] = engine
        return engines


# def chk_ob(core):
#     """
#     Please document this function.
#     """
#     print "Contacts: %r" % core.adb.contacts.keys()
# #    print dir(ui.cli)


def main():    
    """
    Startup of the application.
    """
    global core
    core = Core()
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

