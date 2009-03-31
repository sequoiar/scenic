#!/usr/bin/env python
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
Firewire (ieee1394) bus reset utility.

Useful when a device such as a firewire audio interface needs to be reset.

Make sure /usr/local/bin/firereset is installed.

On Ubuntu, if your user is not in the 'disk' group:

 sudo modprobe raw1394
 sudo gpasswd -a `whoami` disk
"""
from miville.utils.commands import *
from miville.utils import log
from miville.errors import CommandNotFoundError

log = log.start('debug', 1, 0, 'firewire')

_api = None

def firewire_bus_reset(caller):
    """
    Resets the firewire (ieee1394) bus.
    """
    # TODO: add bus number ?
    command = ["firereset"]
    callback = on_commands_results
    extra_args = None
    try:
        single_command_start(command, callback, extra_args, caller)
    except CommandNotFoundError, e:
        log.error("You must install firereset %s" % (e.message))
    
def on_commands_results(results, commands, extra_arg=None, caller=None):
    """
    Called once a bunch of child processes are done.
    """
    for i in range(len(results)):
        result = results[i]
        success, results_infos = result
        command = commands[i]
        
        if isinstance(results_infos, failure.Failure):
            log.error(results_infos.getErrorMessage())
        else:
            stdout, stderr, signal_or_code = results_infos
            if success:
                _api.notify(caller, "Successfully reset the firewire bus.", "info")
            else:
                _api.notify(caller, "Could not reset the firewire bus.", "info")

def start(api):
    global _api
    _api = api

