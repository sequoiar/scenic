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
JACK driver

Attributes of its (jackd) devices: 
* sample rate
* buffer size
"""

import os
from twisted.internet import utils
from twisted.internet import defer
from twisted.python import procutils
from scenic import logger

log = logger.start(name="jackd")

def _parse_jack_info(text):
    """
    Parses the results of the jack-info command.
    
    Returns a list of dict : 
    [{
    'period': 1024,
    'rate': 44100, 
    'latency': 32
    }]
    @rettype: list
    """
    #FIXME: I think jack-info currently only supports reporting 
    # infos one a single JACK server. It's rather rare to see someone
    # using more than one. (it typically needs an audio device for each)
    #FIXME: we don't have the nperiod
    #FIXME: we don't have the backend
    #FIXME: we don't know if jackd is zombie
    
    ret = []
    in_system_capture = False
    in_system_playback = False
    max_system_capture = 0.0
    max_system_playback = 0.0
    
    for line in text.splitlines():
        line = line.strip()
        if line.startswith("JACK server not running"):
            break
        else:
            if len(ret) == 0:
                ret.append({})
            if line.startswith("system:capture"):
                in_system_capture = True
                in_system_playback = False
            elif line.startswith("system:playback"):
                in_system_capture = False
                in_system_playback = True
            elif line.startswith("port latency"):
                pass
            elif line.startswith("total latency"):
                if in_system_capture:
                    val = float(line.split()[3])
                    if val > max_system_capture:
                        max_system_capture = val
                elif in_system_playback:
                    val = float(line.split()[3])
                    if val > max_system_playback:
                        max_system_playback = val
                else:
                    pass
            elif line.startswith("buffer-size"):
                ret[0]["period"] = int(line.split()[1])
            elif line.startswith("samplerate"):
                ret[0]["rate"] = int(line.split()[1])
    if len(ret) > 0:
        ret[0]["latency"] = max_system_playback + max_system_capture
    return ret

def jackd_get_infos():
    """
    Calls jack-info to retrieve info about jackd servers. 
    
    Returns a Deferred whose result is list of dict: 
    [{ 
    'period': 1024,
    'rate': 44100, 
    'latency': 32
    }]
    @rtype: Deferred
    """
    def _cb(text, deferred):
        #print text
        ret = _parse_jack_info(text)
        deferred.callback(ret)
        
    def _eb(reason, deferred):
        deferred.errback(reason)
        print("Error listing jackd servers: %s" % (reason))
    
    command_name = "jack-info"
    args = []
    try:
        executable = procutils.which(command_name)[0] # gets the executable
    except IndexError:
        return defer.fail(RuntimeError("Could not find command %s" % (command_name)))
    deferred = defer.Deferred()
    d = utils.getProcessOutput(executable, args=args, env=os.environ, errortoo=True) # errortoo puts stderr in output
    d.addCallback(_cb, deferred)
    d.addErrback(_eb, deferred)
    return deferred

# TODO: add some example in __main__ here.

