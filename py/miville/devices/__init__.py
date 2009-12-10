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

# drivers
from miville.devices import v4l2
from miville.devices import jackd
from miville.devices import x11

def start(api):
    try:
        v4l2.start(api)
    except CommandNotFoundError, e:
        api.notify(api, e.message, "error")
    try:
        jackd.start(api)
    except CommandNotFoundError, e:
        api.notify(api, e.message, "error")
    try:
        x11.start(api)
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
