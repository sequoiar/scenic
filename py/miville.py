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


# Twisted imports
from twisted.internet import reactor, task

# App imports
import ui
import streams
from utils import log, Subject
import addressbook
import settings


class Core(Subject):
    """Main class of the application and representing the 'Model' in the MVC"""
        
    def __init__(self):
        Subject.__init__(self)
        self.uis = None
        self.startup()
    
    def startup(self):
        self.load_uis()
        self.adb = addressbook.AddressBook('sropulpof')
        self.curr_setting = settings.load()
        self.api = ui.ControllerApi(self)
        
        
        
    def load_uis(self):
        self.uis = ui.load(ui.find_all())
        for mod in self.uis:
            try:
                mod.start(self)
            except:
                log.error('Unable to start UI module %s.' % mod.__name__)



def chk_ob(core):
    print "Obs: %r" % core.observers.valuerefs()
#    print dir(ui.cli)
                    

def main():    
    core = Core()
    l = task.LoopingCall(chk_ob, core)
    l.start(1.0, False)
    reactor.run()


if __name__ == '__main__':
    log.start()
    log.info('Starting Miville (or Sropulpof?)...')
    main()
