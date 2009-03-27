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

"""
This module is a regroupment of different functions that can be use
by any other modules.
"""

# System imports
import sys
import os

# Twisted imports
from twisted.python.modules import getModule
from twisted.python.filepath import FilePath

# App import
from utils import log
from errors import InstallFileError
from i18n import to_utf

log = log.start('debug', 1, 0, 'common')


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
        except Exception, err:
            log.error('Unable to load the module %s. Error: %s' % (mod.name, err))
        else:
            loaded_mods.append(loaded_mod)
    return loaded_mods
    
def find_callbacks(obj, prefix=None):
    """
    Find all methods that there name start with the given prefix and return
    a dictionnary of theses methods where the key is the name of the method
    and the value is the method object.
    
    If no prefix is specify, the function will return all methods that start by
    one underscore (_) but not by two.
    """
    callbacks = {}
    if prefix:
        for attr in vars(obj.__class__).keys():
            if attr.startswith(prefix): 
                callbacks[attr[len(prefix):]] = getattr(obj, attr)
    else:
        for attr in vars(obj.__class__).keys():
            if attr[0] == '_' and attr[1] != '_': 
                callbacks[attr[1:]] = getattr(obj, attr)
    return callbacks

def install_dir(filename, dirname='.sropulpof'):
    """
    Return the complete path.
    (home directory + the application directory + the filename)
    Check before if the directory exist and try to create it if not.
    """
    filename = to_utf(filename).strip()
    dirname = to_utf(dirname).strip()
    if not filename:
        raise InstallFileError, 'File name <%s> is not valid.' % filename
    if not dirname:
        raise InstallFileError, 'Directory name <%s> is not valid.' % dirname
    dirpath = os.path.join(os.environ['HOME'], dirname)
    if not os.path.isdir(dirpath):
        try:
            os.makedirs(dirpath)
        except:
            log.warning('Could not create the directory %s.' % dirpath)
            raise InstallFileError, 'Could not create the directory %s.' % dirpath
    return os.path.join(dirpath, filename)
     

