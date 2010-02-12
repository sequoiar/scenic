#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.

"""
Main of the application.
"""
import sys
from optparse import OptionParser
from twisted.internet import gtk2reactor
gtk2reactor.install() # has to be done before importing reactor
from twisted.internet import reactor
from twisted.internet import error
from twisted.python import log

from scenic import application
from scenic import configure

def start_logging_to_stdout():
    log.startLogging(sys.stdout)

def run():
    # command line parsing
    parser = OptionParser(usage="%prog", version=str(configure.VERSION))
    parser.add_option("-k", "--kiosk", action="store_true", help="Run in kiosk mode")
    parser.add_option("-f", "--fullscreen", action="store_true", help="Run in fullscreen mode")
    (options, args) = parser.parse_args()
    start_logging_to_stdout()
    try:
        app = application.Application(kiosk_mode=options.kiosk, fullscreen=options.fullscreen)
    except error.CannotListenError, e:
        print("There must be an other Scenic running.")
        print(str(e))
    else:
        try:
            reactor.run()
        except KeyboardInterrupt:
            pass
