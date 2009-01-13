#!/usr/bin/env python

from devices import *
# TODO: from utils import find_modules, load_modules

# drivers
from v4l import Video4LinuxDriver

def load_drivers(api):
    """
    api is the ControllerApi object. 
    
    loads all drivers modules from the packages
    
    TODO: fully implement this function.
    (should throw an error in this current state.)
    """
    # TODO !!!
    driver_managers = {}
    for driver_kind in ('video', 'audio', 'data'):
        #driver_managers['driver_kind'] = DriversManager()
        modules = common.load_modules(common.find_modules(driver_kind))
        for module in modules:
            name = module.__name__.rpartition('.')[2]
            try:
                module.start(api)
            except:
                log.error('Connector \'%s\' failed to start.' % name)
            else:
                drivers[name] = module
                log.info('Connector \'%s\' started.' % name)
    return connectors
