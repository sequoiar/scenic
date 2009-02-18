#!/usr/bin/env python
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

from twisted.internet.error import CannotListenError

class Core(Subject):
    """Main class of the application and containing the 'Model' in the MVC"""
        
    def __init__(self):
        Subject.__init__(self)
        self.uis = None
        self.api = None
        self.startup()

    def startup(self):
        """
        Actually does the job that should be done in __init__
        (defines attributes and does the startup routine.)

        Deprecated (IMHO) aalex
        Should be removed and merged to __init__

        If Miville is started by any argument on the CLI, its basic connector will listen on port 
        37055 instead of the default 37054. Useful for debugging.
        """
        # TODO: merge with __init__

        self.api = api.ControllerApi(self.notify)
        devices.start(self.api) # api as an argument
        self.load_uis()
        self.adb = addressbook.AddressBook('sropulpof.adb', self.api)
        self.engines = self.find_engines()
        # create the settings collection
        self.settings = settings.Settings()
        
        #self.curr_setting = self.settings.select()
        self.connectors = connectors.load_connectors(self.api)
        self.com_chan_port = 37054
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



def chk_ob(core):
    """
        I guess this is a function to print some variables 
    """
    print "Contacts: %r" % core.adb.contacts.keys()
#    print dir(ui.cli)

core = None

def main():    
    """
    Startup of the application.
    """
    global core
    core = Core()
#    l = task.LoopingCall(chk_ob, core)
#    l.start(2.0, False)

def exit():
    
    """on application exit"""
    if core.adb != None:
        core.adb.write(False)


if __name__ == '__main__':
    log.start()
    log.info('Starting Sropulpof...')
    try:
        main()
        reactor.run()
    except CannotListenError, e:
        log.error(str(e))
	
    exit()       
