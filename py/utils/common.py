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


# System imports
import sys

# Twisted imports
from twisted.python.modules import getModule
from twisted.python.filepath import FilePath

# App import
from utils import log


def get_def_name(level=2):
    """
    Finds who called the caller of this function
    """
    return sys._getframe(level).f_code.co_name

    
def find_modules(kind):
    """
    Find all the different modules of this kind available
    """
    mods = []
    all_mods = getModule(kind).iterModules()
    for mod in all_mods:
        if mod.isPackage() and not FilePath(mod.filePath.dirname() + '/off').exists():
            mods.append(mod)
    return mods

def load_modules(mods):
    """
    Load/import all the different user interfaces available
    """    
    loaded_mods = []
    for mod in mods:
        try:
            loaded_mod = mod.load()
            log.info('%s module loaded.' % mod.name)
        except:
            log.error('Unable to load the module %s' % mod.name)
        else:
            loaded_mods.append(loaded_mod)
    return loaded_mods
    
def find_callbacks(obj):
    callbacks = {}
    for attr in dir(obj):
        if attr[0] == '_' and attr[1] != '_': 
            callbacks[attr[1:]] = getattr(obj, attr)
    return callbacks
