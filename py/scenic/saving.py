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
Custom state saving tools for Scenic.
"""

import os

# JSON import:
try:
    import json # python 2.6
except ImportError:
    import simplejson as json # python 2.4 to 2.5
try:
    _tmp = json.loads
except AttributeError:
    import warnings
    warnings.warn("Use simplejson, not the old json module.")
    sys.modules.pop('json') # get rid of the bad json module
    import simplejson as json

class ConfigStateSaving(object):
    """
    Loads/saves configuration options to a file.
    """
    def __init__(self, config_file_path):
        self._config_path = config_file_path
        if os.path.isfile(self._config_path):
            self._load()
        else:
            dir_name = os.path.dirname(config_file_path)
            if not os.path.isdir(dir_name):
                os.makedirs(dir_name)
            self.save()

    def save(self):
        """
        Saves the configuration options to a file.
        """
        # TODO: Comments out the options that have not been changed from default.
        config_str = "# Configuration written by scenic" # %(app)s %(version)s\n" % {'app': APP_NAME, 'version': __version__}
        for c in dir(self): # FIXME: use smart var names
            if c[0] != '_' and c != 'save': # FIXME: do not save methods !
                inst_attr = getattr(self, c)
                config_str += "\n" + c + "=" + str(inst_attr)
        config_file = file(self._config_path, "w")
        config_file.write(config_str)
        config_file.close()

    def _load(self):
        # FIXME: use JSON, please.
        print "Loading configuration file %s" % (self._config_path)
        config_file  = file(self._config_path, "r")
        for line in config_file:
            line = line.strip()
            if line and line[0] != "#" and len(line) > 2:
                try:
                    tokens = line.split("=")
                    k = tokens[0].strip()
                    if not hasattr(self, k):
                        print "Unknown configuration attribute: %s" % (k)
                    else:
                        cast = type(getattr(self, k))
                        v = tokens[1].strip()
                        if v.isdigit():
                            v = int(v)
                        elif cast == bool:
                            v = v == 'True' # FIXME
                        else:
                            v = cast(v)
                        setattr(self, k, v)
                        print("Config: %s = %s (%s)" % (k, v, type(v).__name__))
                except Exception, e:
                    print str(e)
        config_file.close()

class AddressBook(object):
    """
    READING & WRITING ADDRESS BOOK FILE 
    """
    def __init__(self):
        self.current_contact_is_new = False
        self.contact_list = []
        self.selected = 0
        #FIXME: do not hard code
        self.contacts_file_name = os.path.join(os.environ['HOME'], '.scenic/contacts.json')
        self.SELECTED_KEYNAME = "selected:" # FIXME
        self.load()

    def load(self):
        print("Loading addressbook.")
        if os.path.isfile(self.contacts_file_name):
            self.contact_list = []
            ad_book_file = file(self.contacts_file_name, "r")
            kw_len = len(self.SELECTED_KEYNAME)
            for line in ad_book_file:
                if line[:kw_len] == self.SELECTED_KEYNAME:
                    self.selected = int(line[kw_len:].strip())
                    print("Loading selected contact: %s" % (self.selected))
                else:
                    try:
                        print("Loading contact %s" % (line.strip()))
                        d = json.loads(line)
                        for k in ['address', 'port', 'name']:
                            if not d.has_key(k):
                                raise RuntimeError("The contacts in %s have a wrong format! The key %s is needed." % (self.contacts_file_name, k))
                        for k, v in d.iteritems():
                            if type(v) is unicode:
                                v = str(v) # FIXME
                        self.contact_list.append(d)
                    except Exception, e:
                        print "ERROR in addresbook:", str(e)
                        raise
            ad_book_file.close()

    def save(self):
        if ((os.path.isfile(self.contacts_file_name)) or (len(self.contact_list) > 0)):
            try:
                ad_book_file = file(self.contacts_file_name, "w")
                for contact in self.contact_list:
                    ad_book_file.write(json.dumps(contact) + "\n")
                if self.selected:
                    ad_book_file.write(self.SELECTED_KEYNAME + str(self.selected) + "\n")
                ad_book_file.close()
            except Exception, e:
                print "Cannot write Address Book file. %s" % (e)
