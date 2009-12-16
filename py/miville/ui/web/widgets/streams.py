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
from miville.streams.conf import ConfError

log = log.start('info', 1, 0, 'web_strm')

CUSTOM_PROFILE_NAME = "Custom"

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
        Javascript wants to update the list of profiles
        Called from the javascript widget.
        """
        self.api.list_profiles(self)
        return False
        
    def rc_set_profile(self, contact_name, profile_id):
        """
        Javascript wants to choose an other profile for a contact

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
        DEFAULT_PROFILE = 0 # FIXME : use the one from dont-remember-where.
        contact = self.api.get_contact(contact_name)
        if contact.profile_id is None:
            log.debug("rc_profile_details_for_contact: Contact %s has nop profile set. Using %s" % (contact.name, DEFAULT_PROFILE))
            profile_id = DEFAULT_PROFILE
            contact.profile_id = profile_id
        else:
            profile_id = contact.profile_id
        try:
            _tmp = self.api.config_db.get_profile(profile_id)
        except ConfError, e:
            log.error("Contact %s uses an invalid profile %s. Setting to %s" % (contact.name, profile_id, DEFAULT_PROFILE))
            profile_id = DEFAULT_PROFILE
            contact.profile_id = profile_id
        self._update_details(profile_id)

    def _update_details(self, profile_id):
        """
        Sends details for a given profile to the Javascript widget.
        """
        # build details for the newly selected profile.
        # ----------- all entries -----------
        entries_details = [] # array of dict with keys "name" and "value"
        try:
            entries = self.api.config_db.get_entries_for_profile(profile_id)
        except ConfError, e:
            log.error("_update_details: Error calling db.get_entries_for_profile: %s" % (e.message))
            return # XXX
        log.debug("entries for profile : %s" % (entries))
        for k, v in sorted(entries.items()):
            field = self.api.config_db.get_field(k)
            #log.debug("    name:%s, value:%s" % (field.desc, v))
            entries_details.append({"name":field.desc, "value":v})
        # ------------ all sets (a profile contains one setting chosen for each set) --------
        settings_ids = self.api.config_db.get_profile(profile_id).settings
        sets_details = [] # array of dict with keys "set_name", "settings", "chosen_setting" and "desc". "setting" is a list of dict with keys setting_id and setting_desc.
        sets_names = self.api.config_db.sets.keys()
        sets_names.sort()
        for set_name in sets_names: #TODO: sort by the_set.desc
            the_set = self.api.config_db.sets[set_name]
            _set_infos = {"set_name":set_name, "settings":[], "chosen_setting":0, "desc":the_set.desc}
            for setting_id in the_set.get_all():
                is_selected = False
                setting_desc = self.api.config_db.get_setting(setting_id).desc
                if setting_id in settings_ids: # this it the chosen one in set!
                    _set_infos["chosen_setting"] = setting_id
                    is_selected = True
                _set_infos["settings"].append({"setting_id": setting_id, "setting_desc": setting_desc, "is_selected":is_selected})
            sets_details.append(_set_infos)
        #log.debug("Sets details: %s" % (sets_details))
        self.callRemote("update_details", entries_details, sets_details, profile_id)
        # TODO: set setting value for each set:
        #for set_name, s in db.sets.iteritems():
        #    print("Set %s is choice between either:" % (set_name))
        #    for setting_id in s.get_all():
        #        print("    - %s" % (db.get_setting(setting_id).name))
        
    def rc_set_value_of_set_for_profile(self, profile_id, set_name, setting_id, contact_name):
        """
        Called by javascript when the user changes the value of a profile's set setting.
        
        Might create a profile named "custom" if called for a profile that is not yet custom.
        """
        profile_id = int(profile_id)
        setting_id = int(setting_id)
        db = self.api.config_db
        profile = db.get_profile(profile_id)
        setting = db.get_setting(setting_id)
        if profile.name.startswith(CUSTOM_PROFILE_NAME):
            log.debug("rc_set_value_of_set_for_profile: Add setting %s:%s to profile %s:%s" % (setting.id, setting.name, profile.id, profile.name))
            db.add_setting_to_profile(profile_id, int(setting_id))
        else:
            custom_name = "%s profile for %s" % (CUSTOM_PROFILE_NAME, contact_name)
            new_profile = db.duplicate_profile(profile_id, name=custom_name)
            log.debug("rc_set_value_of_set_for_profile: New profile %s:%s" % (new_profile.id, new_profile.name))
            db.add_setting_to_profile(new_profile.id, setting_id)
            log.debug("rc_set_value_of_set_for_profile: Set contact %s to use profile %s" % (contact_name, new_profile.name))
            self.api.modify_contact(self, contact_name, profile_id=new_profile.id)
            self.rc_update_profiles()

    def cb_list_profiles(self, origin, data): 
        """
        The API gives responds with the list of profiles.
        
        when 'list_profiles' notification occurs
        """
        js_profiles = []
        profiles = data
        for profile in profiles:
            log.debug("cb_list_profiles: %s %s" % (profile.id, profile.name))
            add_it = True
            if profile.name.startswith(CUSTOM_PROFILE_NAME):
                log.debug("cb_list_profiles: Is a CUSTOM")
                delete = True
                for contact in self.api.addressbook.contacts.itervalues():
                    if contact.profile_id == profile.id:
                        log.debug("cb_list_profiles: Not deleting it since contact %s uses it." % (contact.name))
                        delete = False
                if delete:
                    add_it = False
                    log.warning("Deleting profile %s for all users browsing the web interface !!" % (profile.name)) # FIXME
                    self.api.config_db.delete_profile(profile.id)
            if add_it:
                log.debug("cb_list_profiles: appending profile %s." % (profile.name))
                js_profiles.append({
                    'id':profile.id, 
                    'name':profile.name
                    })
        self.callRemote('update_profiles', js_profiles)

    expose(locals())

