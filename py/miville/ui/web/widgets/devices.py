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
import os
import pprint

#App imports
from miville.ui.web.web import Widget, expose
from miville.utils import log
from miville.utils.i18n import to_utf
from miville.errors import *

log = log.start('debug', 1, 0, 'web_devices')

log.debug("Hello from the devices widget python module.")

class Devices(Widget):
    """
    Web widget for managing devices

     * rc_* methods are called from javascript
     * cb_* methods are Python notification from the API

    Example: (sequence of events)
     * js: devices_list_all()
     * py: rc_devices_list_all()
     * py: api.devices_list_all()
     * py: cb_devices_list_all(caller, data)
     * js: rc_devices_list_all(devs_list)
    """
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

    def cb_devices_list_all(self, origin, data):
        """
        :data: list of devices
        they all belong to the same driver.
        """
        log.debug('Got answer from api.devices_list_all')
        VERY_VERBOSE = False
        devs = []
        #if origin is self:
        if origin is self:
            if len(data) != 0:
                for device in data:
                    dr_kind = device.driver.kind
                    dr_name = device.driver.name
                    dev_name = device.name
                    attributes = device.attributes.values()
                    attr_list = []
                    for attr in attributes:
                        a_name = attr.name
                        a_value = attr.get_value()
                        a_kind = attr.kind # int, string, boolean, options
                        if a_kind == 'options':
                            a_opts = "options=%s" % (attr.options)
                        else:
                            a_opts = "default=%s" % (attr.default)
                        attr_list.append({'name':a_name, 'value':a_value, 'kind':a_kind, 'options':a_opts})
                    d = {"dr_kind":dr_kind, "dr_name":dr_name, "dev_name":dev_name, "attributes":attr_list}
                    devs.append(d)
                    log.debug("device : %s" % (d))
            self.callRemote('rc_devices_list_all', devs)

    expose(locals())
