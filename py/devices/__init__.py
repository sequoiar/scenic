#!/usr/bin/env python

# import subdir modules

# TODO : import sub files

# for d in ['video', 'audio']:
# modules = find_modules('video').
# load_modules(modules)

#from Driver import *
#from Device import *

#from video.Video4linuxDriver import *

from devices import *
#from utils import find_modules, load_modules
# TODO: use find_modules and loadmodules from utils.common

# ------------------------------------- copy&pasted:
# Twisted imports
from twisted.python.modules import getModule
from twisted.python.filepath import FilePath

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




for d in ['video', 'audio']:
    #modules = find_modules(d)
    #load_modules(modules)
    pass
