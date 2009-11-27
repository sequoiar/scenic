#!/usr/bin/env python
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
from twisted.python import failure

#App imports
from miville.ui.web.web import expose
from miville.ui.web.web import Widget
from miville.utils import log
from miville.utils.i18n import to_utf
from miville.errors import *

log = log.start('info', 1, 0, 'web_strm')

class Streams(Widget):
    """
    Web widget to choose settings and start streaming audio/video.
    """
    def rc_start_streams(self, contact):
        """
        Starts the streams

        Called from the javascript widget.
        :param contact: unicode name of the contact
        """
        # self.api.select_contact(self, contact)
        self.api.notify(self, contact, 'will_start_streams_with_contact') # XXX FIXME HACK sending this for addressbook web widget.
        deferred = self.api.start_streams(self, contact)
        #def _cb(result, self):
        #deferred.addCallback(_cb, self)
        return False
        
    def rc_stop_streams(self, contact):
        """
        Javascripts wants to stop the streams
        :param contact: unicode  contact name

        Called from the javascript widget.
        """
        deferred = self.api.stop_streams(self, contact)
        return False
        
    def rc_update_settings(self):
        """
        Javascript wants to update the settings

        Called from the javascript widget.
        """
        self.api.list_profiles(self)
        return False
        
    def rc_set_setting(self, contact, setting):
        """
        Javascript wants to modify the settings for a contact

        Called from the javascript client.
        :param contact: unicode  contact name
        :param setting: int Profile ID
        """
        # set the profile_id for contact.
        self.api.modify_contact(self, contact, profile_id=setting)
        
        # build details for the newly selected profile.
        details = [] # array of dict with keys "name" and "value"
        profile_id = int(setting)
        entries = self.api.config_db.get_entries_for_profile(profile_id)
        log.debug("entries for profile : %s" % (entries))
        for k, v in sorted(entries.items()):
            field = self.api.config_db.get_field(k)
            log.debug("    name:%s, value:%s" % (field.name, v))
            details.append({"name":field.name, "value":v})
        self.callRemote("update_details", details)
        
        return False
        
    def cb_list_profiles(self, origin, data): #TODO:rename this ugly method name
        """
        The API gives responds with the list of profiles.
        
        when 'list_profiles' notification occurs
        """
        preset_settings = []
        user_settings = []
        profiles = data
        for profile in profiles:
            preset_settings.append({
                'id':profile.id, 
                'name':profile.name
                })
        self.callRemote('update_settings', preset_settings, user_settings)

    #def cb_remote_started_streams(self, origin, data):
    #    """
    #    The API tells us that the remote Alice has started streams with us. (Bob)
    #    
    #    sets the contact stream_state to streaming when started from remote.
    #    """
    #    log.debug("cb_remote_started_streams %s %s" % (origin, data))
    #    if isinstance(data, failure.Failure):
    #        pass
    #    else:
    #        pass

    #def cb_remote_stopped_streams(self, origin, data):
    #    """
    #    sets the contact stream_state to stopped when stopped from remote.
    #    """
    #    log.debug("cb_remote_stopped_streams %s %s" % (origin, data))
    #    if isinstance(data, failure.Failure):
    #        pass
    #    else:
    #        pass
    
    expose(locals())

