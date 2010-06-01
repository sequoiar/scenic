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
Glade RAD tool file loading and widgets factory.
"""
import os
import sys
import gtk.glade
from twisted.internet import reactor
from scenic import configure
from scenic.internationalization import _

def get_widgets_tree():
    """
    Returns a L{gtk.glade.XML} object.
    
    Keep in mind that gtk.glade automatically caches XML trees. So don't try
    any complex tricks to reuse XML trees if you have to create the same UI
    multiple times. The correct thing to do is simply to instantiate the XML
    multiple times with the same parameters. 
    """
    # Set the Glade file
    glade_file = os.path.join(configure.GLADE_DIR, 'scenic.glade')
    if os.path.isfile(glade_file):
        glade_path = glade_file
    else:
        text = _("Error : Could not find the Glade file %(filename)s. Exitting.") % {"filename": glade_file}
        print(text)
        def _exit_cb(unused_result):
            print("Exiting")
            if reactor.running:
                reactor.stop()
        # Recursive imports in Python are really badly managed.
        # I hope this will be fixed in Python 3.0
        from scenic import dialogs
        dialogs.ErrorDialog.create(text, parent=None).addCallback(_exit_cb)
        # FIXME: raise error or what? Exitting for now
        sys.exit(1)
    return gtk.glade.XML(glade_path, domain=configure.APPNAME)
