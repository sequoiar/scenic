#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
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
The devices packages contains module to manage audio/video devices ong GNU/Linux.

It allows to list devices for each driver kind, and change their attributes. 

Some data drivers (such as MIDI) should be here later on.
"""
import sys

from miville.devices.devices import *
from miville.errors import CommandNotFoundError
# TODO: from miville.utils import find_modules, load_modules

# drivers
from miville.devices import v4l2
from miville.devices import jackd

def start(api):
    try:
        v4l2.start(api)
    except CommandNotFoundError, e:
        api.notify(api, e.message, "error")
    try:
        jackd.start(api)
    except CommandNotFoundError, e:
        api.notify(api, e.message, "error")

def stop():
    """
    The idea is to call the destructors of every drivers in order to 
    remove any reactor.callLater
    """
    try:
        del devices.managers['audio'].drivers['jackd']
        del devices.managers['video'].drivers['v4l2']
    except:
        print sys.exc_info()

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



