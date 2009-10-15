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
"""
This module is a regroupment of different functions that can be use
by any other modules.
"""
# System imports
import sys
import os
import warnings

# Twisted imports
from twisted.python.modules import getModule
from twisted.python.filepath import FilePath

# App import
from miville.utils import log
from miville.errors import InstallFileError
from miville.utils.i18n import to_utf

log = log.start('debug', 1, 0, 'common')

# this path is overriden by the core at startup !
MIVILLE_HOME = os.path.expanduser('~/.miville')

_allocated_ports = []

class PortNumberGenerator(object):
    """
    Allocates unique TCP/UDP port numbers.
    using a global variable to store the already allocated ports.
    """
    #TODO: stay within a max port number range.
    #TODO: use a set instead of a list to make sure they are unique?
    #FIXME: this will eventually overflow. 
    #TODO: handle case when it overflows. Use an other algoruthm
    def __init__(self, first_port, increment):
        """
        :param first_port: First port offset to allocate ports
        :param increment: space between each allocated port.
        """
        self.first_port = first_port
        self.current_port = None
        self.increment = increment
        warnings.warn("PortNumberGenerator will eventually overflow.")

    def get_current_port(self):
        """
        Returns the most recenlty allocated port number.
        None if None has been allocated so far.
        """
        return self.current_port
    
    def get_all(self):
        global _allocated_ports
        return _allocated_ports
    
    def generate_new_ports(self, count):
        """
        Generates many new port numbers.
        returns a list of allocated numbers.
        """
        ports = []
        for i in range(count):
            x = self.generate_new_port()
            ports.append(x)
        return ports
        
    def free_port(self, port_num):
        """
        Deletes a port number from the list of allocated ports.
        """
        # TODO: better error handling. maybe a better exception type
        global _allocated_ports
        if port_num not in _allocated_ports:
            raise Exception("Port %d not allocated !" % (port_num))
        else:
            _allocated_ports.pop(_allocated_ports.index(port_num))
            self.current_port = None # TODO: anything better to do here?
            # TODO: decrement next port index ???
            
    def generate_new_port(self):
        """
        Allocates a single new port number 
        """
        global _allocated_ports 
        found = False
        candidate = self.current_port
        if not self.current_port:
            candidate = self.first_port - self.increment
        while not found:
            candidate += self.increment
            if candidate not in _allocated_ports:
                self.current_port = candidate
                _allocated_ports.append(self.current_port)
                found = True
        return self.get_current_port()


def string_to_number(value):
    """
    Returns a number or None if the string
    is not a integer or a float 
    """
    if value.isdigit():
        return int(value)
    try:
        return float(value)
    except:
        return None

def get_def_name(level=2):
    """
    Finds who called the caller of this function
    """
    return sys._getframe(level).f_code.co_name
    
def find_modules(kind):
    """
    Find all the different modules/packages of a kind available
    
    This is an implementation of a system for dynamically loaded python plugins.
    
    IMPORTANT : If a file named "off" is in a package directory, it doesn't load that package.

    :param kind: str The name of the package to look in.
    """
    # FIXME: this is not very elegant, and not python-like at all. (sorry etienned)
    # FIXME: it caused us a lot of troubles so far
    mods = []
    all_mods = getModule('miville.' + kind).iterModules()
    for mod in all_mods:
        if mod.isPackage() and not FilePath(mod.filePath.dirname() + '/off').exists():
            mods.append(mod)
    return mods

def load_modules(mods):
    """
    Load/import all the different user interfaces available
    """    
    # FIXME : lots of bugs due to this function !! 
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

def install_dir(filename, dirname=None):
    """
    Return the complete path of the miville configuration directory. (~/.miville)
    
    (home directory + the application directory + the filename)
    Check before if the directory exist and try to create it if not.
    """
    global MIVILLE_HOME

    if dirname is None:
        dirname = MIVILLE_HOME 
    filename = to_utf(filename).strip()
    dirname = to_utf(dirname).strip()
    if not filename:
        raise InstallFileError, 'File name <%s> is not valid.' % (filename)
    if not dirname:
        raise InstallFileError, 'Directory name <%s> is not valid.' % (dirname)
    dirpath = dirname # XXX removed $HOME from here. os.path.join(os.environ['HOME'], dirname)
    if not os.path.isdir(dirpath):
        try:
            os.makedirs(dirpath)
            log.info('creating directory %s' % (dirpath))
        except:
            log.warning('Could not create the directory %s.' % (dirpath))
            raise InstallFileError, 'Could not create the directory %s.' % dirpath
    return os.path.join(dirpath, filename)
     
