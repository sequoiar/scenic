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
V4L2 Video Devices Utilities.
"""

from twisted.internet import reactor
from twisted.internet import utils

def _parse_v4l2_ctl_list_inputs(lines):
    """
    Parses the output of `v4l2-ctl --list-inputs -d /dev/video0`
    
    Returns a list with all input names.
    Their index is their number.
    """
    inputs = []
    for line in lines:
        if line.startswith('\t'):
            splitted = line.strip('\t').split(':')
            try:
                key = splitted[0].strip()
                value = splitted[1].strip()
            except IndexError:
                pass
            else:
                if key == 'Name':
                    inputs.append(value.strip())
    # log.debug('v4l2 inputs: %s' % (inputs))
    return inputs

def list_v4l2_cameras():
    """
    @rettype Deferred
    """
    commands.append(['v4l2-ctl', '--all', '-d', name])
    raise Exception()
    
def list_v4l2_inputs(device_name="/dev/video0"):
    """
    Calls the Deferred with the list of the names of inputs as argument. 
    The input numbers are the positions in the list.
    @param device_name: String like /dev/video0 or /dev/video1, etc.
    @rettype: Deferred
    """
    def _cb(lines, deferred):
        ret = _parse_v4l2_ctl_list_inputs(lines)
        deferred.callback(ret)
        
    def _eb(reason, deferred):
        deferred.errback(reason)
    
    command_name = "v4l2-ctl"
    args = ['--list-inputs', '-d', name]
    try:
        executable = procutils.which(command_name)[0] # gets the executable
    except IndexError:
        return defer.fail(RuntimeError("Could not find command %s" % (command_name)))
    deferred = defer.Deferred()
    d = utils.getProcessOutput(executable, args=args, env=os.environ)
    d.addCallback(_cb, deferred)
    d.addErrback(_eb, deferred)
    return deferred
    

