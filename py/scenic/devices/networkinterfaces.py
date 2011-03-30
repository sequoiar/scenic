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
Tools to list network interfaces.
"""
import os
from twisted.internet import threads
from twisted.internet import utils
from twisted.internet import defer
from twisted.internet import reactor
from twisted.python import procutils

def _parse_ifconfig(text):
    ret = []
    for line in text.splitlines():
        if "inet" in line and not "inet6" in line:
            ip = line.strip().split(" ")[1].split(":")[1]
            valid = True
            if ip.startswith("127."): # loopback or local hostname
                valid = False
            elif ip.startswith("169.254."): # zeroconf address
                valid = False
            # TODO: check IP class
            if valid:
                ret.append(ip)
    return ret

def list_network_interfaces_addresses():
    """
    Lists network interfaces IP.
    @rtype: Deferred
    """
    def _cb(result, deferred):
        #print 'cb', result
        ret = _parse_ifconfig(result)
        deferred.callback(ret)
        return None
    def _eb(reason, deferred):
        print("Error calling ifconfig.")
        print(reason)
        deferred.errback(reason)
        return None
    command_name = "ifconfig"
    args = []
    try:
        executable = procutils.which(command_name)[0] # gets the executable
    except IndexError:
        return defer.fail(RuntimeError("Could not find command %s" % (command_name)))
    deferred = defer.Deferred()
    d = utils.getProcessOutput(executable, args=args, env=os.environ)
    d.addCallback(_cb, deferred)
    d.addErrback(_eb, deferred)
    return deferred

if __name__ == "__main__":
    def _cb(result):
        print "result:", result
        reactor.stop()
    d = list_network_interfaces_addresses()
    d.addCallback(_cb)
    reactor.run()
