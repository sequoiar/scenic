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
X11 driver. Used to list X11 displays/screens.

Uses xpyinfo, looping until no more screen/display is found. ::

  xdpyinfo -display :0.0
"""
import subprocess
from twisted.internet import reactor

from miville.devices import devices #FIXME: ugly
from miville.utils import log as logger
from miville.utils import commands
from miville.errors import CommandNotFoundError

log = logger.start('info', True, True, 'x11')

class X11Driver(devices.VideoDriver):
    """
    Miville X11 displays driver
    """
    name = 'x11'
    command_found = False
    
    def prepare(self):
        """
        Returns a Deferred instance? no
        """
        try:
            tmp = commands.find_command('xdpyinfo')
        except CommandNotFoundError, e:
            #self.api.notify(None, "xdpyinfo command not found. Is xorg installed ?", "error")
            log.error("xdpyinfo command not found. Is xorg installed ?")
            raise # catched in devices.py
        else:
            self.command_found = True
        return devices.Driver.prepare(self)
    
    def _on_devices_polling(self, caller=None, event_key=None):
        """
        Get all infos for all devices.
        
        Overrides devices.Device._on_devices_polling
        Must return a Deferred instance.
        """
        if self.command_found:
            displays = _list_x11_displays()
            for display in displays:
                log.debug("Adding display %s" % (display["name"]))
                d = devices.Device(display['name'])
                d.add_attribute(devices.StringAttribute("resolution", display["resolution"]))
                d.add_attribute(devices.StringAttribute("dimensions", display["dimensions"]))
                self._add_new_device(d)
        self._on_done_devices_polling(caller, event_key)

    def on_attribute_change(self, attr, caller=None, event_key=None):
        raise DeviceError("It is not possible to change x11 devices attributes.")

def _list_x11_displays():
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
        log.debug("calling xdpyinfo -display %s" % (display))
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
                log.debug("Found X11 display and screen %s with dimensions = %s" % (screen, dimen))
        if not found_screen_for_display:
            break
    dev_null.close()
    return displays

def start(api):
    """
    Starts this driver.
    
    Called from the core.
    """
    driver = X11Driver()
    driver.api = api
    log.info("Starting the X11 driver.")
    devices.managers['video'].add_driver(driver)
    reactor.callLater(0, driver.prepare)

if __name__ == "__main__":
    print(_list_x11_displays())
