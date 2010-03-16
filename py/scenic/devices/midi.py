#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.
"""
MIDI driver. Used to list MIDI devices.

Uses midistream.
"""
import os
import shlex
from twisted.internet import threads
from twisted.internet import utils
from twisted.internet import defer
from twisted.internet import reactor
from twisted.python import procutils

# $ miditream --list-devices
# List of MIDI devices:
#     Input devices:
#     --------------
#      * input   1            Midi Through Port-0   [closed]
#      * input   3         USB Oxygen 8 v2 MIDI 1   [closed]
#     Output devices:
#     ---------------
#      * output  0            Midi Through Port-0   [closed]
#      * output  2         USB Oxygen 8 v2 MIDI 1   [closed]
#      * output  4                TiMidity port 0   [closed]
#      * output  5                TiMidity port 1   [closed]
#      * output  6                TiMidity port 2   [closed]
#      * output  7                TiMidity port 3   [closed]
#      * output  8                       qjackctl   [closed]

def _parse_miditream_list_devices(text):
    """
    Parses the output of `midistream --list-devices`
    Returns a list of dict with keys "name", "number", "is_input", "is_open".
    @rtype: list
    """
    midi_devices = []
    for line in text.splitlines():
        line = line.strip()
        print line
        if line.startswith('*'):
            device = {
                "is_input": False,
                "name": "",
                "number": 0,
                "is_open": False,
                }
            tokens = shlex.split(line)
            if tokens[1] == "output":
                device["is_input"] = False
            elif tokens[1] == "input":
                device["is_input"] = True
            device["number"] = int(tokens[2])
            device["name"] = tokens[3]
            if tokens[4] == "[closed]":
                device["is_open"] = False
            elif tokens[4] == "[open]":
                device["is_open"] = True
            midi_devices.append(device)
    return midi_devices

def list_midi_devices(verbose=True):
    """
    Twisted wrapper for _list_x11_displays.
    Result is a dict with keys "input" and "output". The value are dict with ID and name for each device.
    @rtype: Deferred
    """
    deferred = defer.Deferred()
    def _cb(text, deferred):
        #print text
        ret = _parse_miditream_list_devices(text)
        deferred.callback(ret)
        
    def _eb(reason, deferred):
        deferred.errback(reason)
        print("Error listing MIDI devices: %s" % (reason))
    
    command_name = "midistream"
    args = ['--list-devices']
    try:
        executable = procutils.which(command_name)[0] # gets the executable
    except IndexError:
        return defer.fail(RuntimeError("Could not find command %s" % (command_name)))
    print "$ %s %s" % (executable, args)
    d = utils.getProcessOutput(executable, args=args, env=os.environ, errortoo=True) # errortoo puts stderr in output
    d.addCallback(_cb, deferred)
    d.addErrback(_eb, deferred)
    return deferred

if __name__ == "__main__":
    def _cb(result):
        print "result:", result
        reactor.stop()
    d = list_midi_devices()
    d.addCallback(_cb)
    reactor.run()

