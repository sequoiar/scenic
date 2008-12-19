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
from twisted.python.modules import getModule
from twisted.python.filepath import FilePath
from utils import log

DEBUG = False
    
def find_all():
    """
    Find all the different user interfaces available
    """
    uis = []
    mods = getModule('ui').iterModules()
    for ui in mods:
        if ui.isPackage() and not FilePath(ui.filePath.dirname() + '/off').exists():
            uis.append(ui)
    return uis

def load(uis):
    """
    Load/import all the different user interfaces available
    """
    loaded_uis = []
    for ui in uis:
        if DEBUG:
            loaded_ui = ui.load()
            log.info('%s module loaded.' % ui.name)
            loaded_uis.append(ui.load())
        else:
            try:
                loaded_ui = ui.load()
                log.info('%s module loaded.' % ui.name)
            except:
                log.error('Unable to load the module %s' % ui.name)
            else:
                loaded_uis.append(ui.load())
    return loaded_uis
    
def find_callbacks(obj):
    callbacks = {}
    for attr in dir(obj):
        if attr[0] == '_' and attr[1] != '_': 
            callbacks[attr[1:]] = getattr(obj, attr)
    return callbacks
