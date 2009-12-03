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
        self.api.notify(self, contact, 'will_start_streams_with_contact') # XXX FIXME HACK sending this for addressbook web widget.
        deferred = self.api.start_streams(self, contact)
        return False
        
    def rc_stop_streams(self, contact):
        """
        Javascripts wants to stop the streams
        :param contact: unicode  contact name

        Called from the javascript widget.
        """
        deferred = self.api.stop_streams(self, contact)
        return False
        
    def rc_update_profiles(self):
        """
        Javascript wants to update the profiles
        Called from the javascript widget.
        """
        self.api.list_profiles(self)
        return False
        
    def rc_set_profile(self, contact_name, profile_id):
        """
        Javascript wants to modify the settings for a contact

        Called from the javascript client.
        :param contact_name: unicode  contact name
        :param profile_id: int Profile ID
        """
        # set the profile_id for contact.
        self.api.modify_contact(self, contact_name, profile_id=int(profile_id))
        self._update_details(int(profile_id))
        return False

    def rc_profile_details_for_contact(self, contact_name):
        """
        Called from js
        Will update profile details for selected contact.
        :param contact_name: unicode
        """
        contact = self.api.get_contact(contact_name)
        if contact.profile_id is None:
            profile_id = 0 #FIXME
        else:
            profile_id = contact.profile_id
        self._update_details(profile_id)

    def _update_details(self, profile_id):
        """
        Sends profile details to Javascript widget.
        """
        # build details for the newly selected profile.
        details = [] # array of dict with keys "name" and "value"
        entries = self.api.config_db.get_entries_for_profile(profile_id)
        log.debug("entries for profile : %s" % (entries))
        for k, v in sorted(entries.items()):
            field = self.api.config_db.get_field(k)
            log.debug("    name:%s, value:%s" % (field.desc, v))
            details.append({"name":field.desc, "value":v})
        self.callRemote("update_details", details)
        
    def cb_list_profiles(self, origin, data): 
        """
        The API gives responds with the list of profiles.
        
        when 'list_profiles' notification occurs
        """
        js_profiles = []
        profiles = data
        for profile in profiles:
            js_profiles.append({
                'id':profile.id, 
                'name':profile.name
                })
        self.callRemote('update_profiles', js_profiles)

    expose(locals())

