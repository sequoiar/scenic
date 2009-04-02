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
Devices settings for Miville.
"""
from miville import devices
from miville.errors import DeviceError
from miville.utils import log

log = log.start('info', 1, 0, 'devices_settings')

class VirtualAttribute(devices.Attribute):
    """
    Attribute used for devices settings.

    This class overrides one method of the Attribute class
    so that when these attributes can be used for settings
    handling, and no more for reflecting the actual state
    of an existing device.

    The StringAttribute, IntAttribute, BooleanAttribute and 
    OptionsAttribute classes defined in this file also
    override their corresponding class from the device module.
    """
    def _on_change(self, caller=None, event_key=None):
        """
        Called when the value changed.

        Does not tell the Device that the value changed, but rather
        simply stores the value.
        """ 
        pass

class StringAttribute(devices.StringAttribute, VirtualAttribute):
    pass
class BooleanAttribute(devices.BooleanAttribute, VirtualAttribute):
    pass
class OptionsAttribute(devices.OptionsAttribute, VirtualAttribute):
    pass
class IntAttribute(devices.IntAttribute, VirtualAttribute):
    pass

class DevicesGroup(object):
    """
    Group of device settings.

    The order of DeviceSettings objects in the device_settings list
    matters. It decides the order in which devices are chosen. 
    For example, if the /dev/video1 video device is before /dev/video0, 
    it will be the chosen one if the settings are set to use only one
    video stream.
    """
    def __init__(self):
        self.id = None
        self.name = None
        self.device_settings = []
        # self.devices_names = []
    
    def apply_modified_attributes(self):
        """
        Applies the attribute changes to the real devices.
        """
        pass

class DeviceSettings(object):
    """
    Settings for a device.

    Can be used for any device. (a v4l2 can be used for /dev/video0 or /dev/video1)
    """
    def list_actual_devices(self, kind="video"):
        """
        Returns list of the actual devices of a kind.

        kinds can be "video", "audio" or "data"
        """
        pass
