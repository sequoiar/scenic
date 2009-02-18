#!/usr/bin/env python

from devices import *
from errors import CommandNotFoundError
# TODO: from utils import find_modules, load_modules

# drivers
import v4l2
import jackd

def start(api):
    try:
        v4l2.start(api)
    except CommandNotFoundError, e:
        api.notify(api, e.message, "error")
    try:
        jackd.start(api)
    except CommandNotFoundError, e:
        api.notify(api, e.message, "error")

# def load_drivers(api):
#     # TODO !!!
#     """
#     api is the ControllerApi object. 
#     
#     loads all drivers modules from the packages
#     
#     TODO: fully implement this function.
#     (should throw an error in this current state.)
#     """
#     driver_managers = {}
#     for driver_kind in ('video', 'audio', 'data'):
#         #driver_managers['driver_kind'] = DriversManager()
#         modules = common.load_modules(common.find_modules(driver_kind))
#         for module in modules:
#             name = module.__name__.rpartition('.')[2]
#             try:
#                 module.start(api)
#             except:
#                 log.error('Connector \'%s\' failed to start.' % name)
#             else:
#                 drivers[name] = module
#                 log.info('Connector \'%s\' started.' % name)
#     return connector



