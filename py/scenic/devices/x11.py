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
X11 driver. Used to list X11 displays/screens.

Uses xpyinfo, looping until no more screen/display is found. ::

  xdpyinfo -display :0.0
"""
import os
import subprocess
from twisted.internet import threads
from twisted.internet import utils
from twisted.internet import defer
from twisted.internet import reactor
from twisted.python import procutils

def _list_x11_displays(verbose):
    """
    Returns a list of X11 display/screen names.
    
    Not using Twisted, since this should be pretty fast.
    It is also a lot easier to debug this way.
    
    Returns a list of dict whose keys are : name, dimensions, resolution
    :rettype: list of dict
    """
    dev_null = open("/dev/null", "wa")
    displays = []
    for i in range(10):
        display = ":%d.0" % (i)
        if verbose:
            print("calling xdpyinfo -display %s" % (display))
        command = "xdpyinfo -display %s" % (display)
        output = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=dev_null).communicate()[0]
        found_screen_for_display = False
        for j in range(10):
            try:
                screen_str = output.split("screen #")[j + 1]
                dimen = screen_str.split("dimensions:")[1].split("\n")[0].strip()
                resolution = screen_str.split("resolution:")[1].split("\n")[0].strip()
            except IndexError:
                break # no display number j+1 found.
            else:
                found_screen_for_display = True
                screen = ":%d.%d" % (i, j)
                displays.append({"name":screen, "dimensions":dimen, "resolution":resolution})
                if verbose:
                    print("Found X11 display and screen %s with dimensions = %s" % (screen, dimen))
        if not found_screen_for_display:
            break
    dev_null.close()
    
    # add current DISPLAY variable if not in the list of detected displays. (most likely because it's a ssh -X session)
    if os.environ.has_key("DISPLAY"):
        display = os.environ["DISPLAY"]
        found_it = False
        for d in displays:
            if d["name"] == display:
                found_it = True
        if not found_it:
            displays.append({"name":display, "dimensions":"unknown", "resolution":"unknown"})
    return displays

def list_x11_displays(verbose=True):
    """
    Twisted wrapper for _list_x11_displays.
    Result is a list of dicts with keys 'name', 'dimensions', 'resolution'.
    @rettype: Deferred
    """
    return threads.deferToThread(_list_x11_displays, verbose)

def xvideo_extension_is_present():
    """
    Checks for XV extension.
    Result is boolean.
    
    @rettype: Deferred
    """
    def _cb(result, deferred):
        ret = True
        for line in result.splitlines():
            if line.find("no adaptors present") != -1: # Hardy
                ret = False
            if line.find("no adaptor present") != -1: # Karmic
                ret = False
        deferred.callback(ret)
        
    def _eb(reason, deferred):
        deferred.errback(reason)

    command_name = "xvinfo"
    try:
        executable = procutils.which(command_name)[0] # gets the executable
    except IndexError:
        return defer.fail(RuntimeError("Could not find command %s" % (command_name)))
    deferred = defer.Deferred()
    d = utils.getProcessOutput(executable, env=os.environ)
    d.addCallback(_cb, deferred)
    d.addErrback(_eb, deferred)
    return deferred
    

if __name__ == "__main__":
    def _cb(result):
        print "result:", result
        reactor.stop()
    d = list_x11_displays()
    d.addCallback(_cb)
    reactor.run()
