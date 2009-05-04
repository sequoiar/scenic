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

#App imports
from miville.ui.web.web import Widget, expose
from miville.utils import log
from miville.utils.i18n import to_utf
from miville.errors import *

log = log.start('debug', 1, 0, 'web_strm')

class Streams(Widget):
    """
    Web widget to choose settings and start streaming audio/video.
    """
    def rc_start_streams(self, contact):
        """
        Starts the streams

        Called from the javascript widget.
        """
        self.api.start_streams(self, contact)
        return False
        
    def rc_stop_streams(self, contact):
        """
        Stops the streams

        Called from the javascript widget.
        """
        self.api.stop_streams(self, contact)
        return False
        
    def rc_update_settings(self):
        """
        Updates the settings

        Called from the javascript widget.
        """
        self.api.list_global_setting(self)
        return False
        
    def rc_set_setting(self, contact, setting):
        """
        Modifies the settings for a contact

        Called from the javascript client.
        """
        self.api.modify_contact(self, contact, setting=setting)
        return False
        
    def cb_list_global_setting(self, origin, data):
        """
        Called from the python API when 'list_global_settings' notification occurs
        """
        preset_settings = []
        user_settings = []
        for id, setting in data[0].items():
            if setting.is_preset:
                preset_settings.append({'id':id, 'name':setting.name}) 
            else:
                user_settings.append({'id':id, 'name':setting.name}) 
            log.debug('LIST GLOB: %s' % id)
            log.debug('LIST GLOB: %s' % setting.name)
            log.debug('LIST GLOB: %s' % setting.is_preset)
        preset_settings.sort(key=lambda x:(x['name'].lower))
        user_settings.sort(key=lambda x:(x['name'].lower))
        self.callRemote('update_settings', preset_settings, user_settings)
        
    def cb_start_streams(self, origin, data):
        """
        Called once miville has started to stream.

        This is to update the streaming status on B's side.
        (whan A initiates the streams, and B is passive)
        """
        log.debug("START_STREAMS " + str(data))
        #  START_STREAMS {'started': True, 'msg': 'streaming started'}
        try:
            started = data['streaming'] is True
            if started:
                log.warning('TODO:self.callRemote()')
            else:
                log.warning('TODO:self.callRemote()')
        except IndexError:
            log.error("IndexError: data in start_streams should be a dict")

    def cb_stop_streams(self, origin, data):
        """
        Called once miville has stropped streaming.
        """
        log.debug("STOP_STREAMS " + str(data))

    expose(locals())
