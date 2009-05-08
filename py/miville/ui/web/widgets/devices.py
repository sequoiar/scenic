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
# along with Miville. If not, see <http://www.gnu.org/licenses/>.

# System import
import time
import pprint

#App imports
from miville.ui.web.web import Widget, expose
from miville.utils import log
from miville.utils.i18n import to_utf
from miville.errors import *

log = log.start('debug', 1, 0, 'web_devices')

class Devices(Widget):
    """
    Web widget for managing devices

     * rc_* methods are called from javascript
     * cb_* methods are Python notification from the API

    Example: (sequence of events)
     * js: devices_list('video', 'v4l2')
     * py: rc_devices_list('video', 'v4l2')
     * py: api.devices_list(caller, 'video', 'v4l2')
     * py: cb_devices_list(caller, data)
     * js: rc_devices_list('video', 'v4l2', devices_list)
    """
    def rc_devices_list(self, kind): # , driver_kind):
        """
        kind = 'audio' or 'video'
        TODO: will notify with 'info' key if there is an error.
        We should have a device_error key.
        """
        log.debug('devices_list %s' % (kind)) # , driver_kind))
        caller = self
        devs = self.api.devices_list(caller, kind)
        return False # we must do this for rc_* methods
    
    def rc_devices_list_all(self):
        """
        TODO: will notify with 'info' key if there is an error.
        """
        log.debug('Will call api.devices_list_all')
        devs = self.api.devices_list_all(self)
        return False
    
    def cb_devices_removed(self, origin, data):
        """These might cause troubles if attributes change too often."""
        log.debug('cb_devices_removed')
        self.api.devices_list_all(self)
    def cb_devices_added(self, origin, data):
        """These might cause troubles if attributes change too often."""
        log.debug('cb_devices_added')
        self.api.devices_list_all(self)
    def cb_device_attributes_changed(self, origin, data):
        """These might cause troubles if attributes change too often."""
        log.debug('cb_device_attributes_changed')
        self.api.devices_list_all(self)

    def cb_devices_list(self, origin, data):
        """
        Devices list for one kind of driver. (video or audio)
        For now, expect only one driver for each kind of driver. 
        There are only v4l2 for video and jackd for audio
        """
        if origin is self:
            if len(data) == 0:
                log.info("No devices to list. What kind was it ?")
            else:    
                msg = ""
                driver_name = data[0].driver.name
                for device in data:
                    msg += "\t%s" % (device.name)
                log.debug("cb_devices_list" + msg)
                devices_list = msg
                self.callRemote('rc_devices_list', driver_name, devices_list)

    def cb_devices_list_all(self, origin, data):
        """
        :data: list of devices
        they all belong to the same driver.
        """
        log.debug('Got answer from api.devices_list_all')
        lines = []
        devs = []
        if origin is self:
            if len(data) == 0:
                lines.append("No devices to list.")
            else:
                for device in data:
                    dr_kind = device.driver.kind
                    dr_name = device.driver.name
                    dev_name = device.name
                    attributes = device.attributes.values()
                    attr_list = []
                    lines.append("%s driver \"%s\": device \"%s\"" % (dr_kind, dr_name, dev_name))
                    for attr in attributes:
                        a_name = attr.name
                        a_value = attr.get_value()
                        a_kind = attr.kind # int, string, boolean, options
                        if a_kind == 'options':
                            a_opts = "options=%s" % (attr.options)
                        else:
                            a_opts = "default=%s" % (attr.default)
                        attr_list.append({'name':a_name, 'value':a_value, 'kind':a_kind, 'options':a_opts})
                        lines.append("    - %15s = %15s     (%s)" % (a_name, a_value, a_kind)) # , a_opts))
                    lines.append('')
                    devs.append({'kind':dr_kind, 'driver_name':dr_name, 'device_name':dev_name, 'attributes':attr_list})
            self.callRemote('rc_devices_list_all', "\n".join(lines), devs)

#     def rc_device_list_attributes(self, driver_kind, driver_name, device_name): 
#         """
#         List of attributes for one device
#         """
#         log.debug('device_list_attributes %s %s %s' % (driver_kind, driver_name, device_name))
#         self.api.device_list_attributes(self, driver_kind, driver_name, device_name): 
#         return False
# 
#     def cb_device_list_attributes(self, origin, data):
#         attributes = data
#         log.debug("started network test" + str(origin) + str(data))
#         self.callRemote('rc_device_list_attributes', attributes)

    expose(locals())

