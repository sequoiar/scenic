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
from optparse import OptionParser

try:
    import pygtk
    pygtk.require("2.0")
except Exception, e:
    print e
try:
    import gtk
except ImportError, e:
    print "Could not load GTK or glade. Install python-gtk2 and python-glade2.", str(e)
    sys.exit(1)
from propulseart import gui

def run():
    # command line parsing
    parser = OptionParser(usage="%prog", version=str(gui.__version__))
    parser.add_option("-k", "--kiosk", action="store_true", dest="kiosk", \
            help="Run maugis in kiosk mode")
    (options, args) = parser.parse_args()
    app = gui.Application(kiosk=options.kiosk)
    gtk.main()
