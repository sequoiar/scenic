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
from scenic import configure
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
    import sys
    warnings.warn("Use simplejson, not the old json module.")
    sys.modules.pop('json') # get rid of the bad json module
    import simplejson as json

def _create_directory_if_it_does_not_exist(dir_path):
    try:
        if not os.path.exists(dir_path):
            os.makedirs(dir_path)
            print('mkdir %s' % (dir_path))
    except OSError, e:
        msg = 'Error creating directories' % (dir_path, e.message)
        print(msg)
        raise RuntimeError(msg)

def _save(file_name, data):
    """
    State saving using JSON

    The data attribute is a dict of basic types.
    (``str``, ``unicode``, ``int``, ``long``, ``float``, ``bool``, ``None``)
    It can contain dicts and lists as well.
    """
    dir_name = os.path.dirname(file_name)
    _create_directory_if_it_does_not_exist(dir_name)
    f = None
    try:
        f = open(file_name, "w")
    except IOError, e:
        raise RuntimeError(e.message)
    else:
        print("Writing data in JSON to %s" % (file_name))
        print("%s" % (data))
        json.dump(data, f, indent=4)
    if f is not None:
        f.close()

def _load(file_name):
    f = None
    try:
        f = open(file_name, "r")
    except IOError, e:
        raise RuntimeError(e.message)
    else:
        try:
            data = json.load(f)
        except ValueError, e:
            raise RuntimeError("Error in JSON formatting: %s" % (e.message))
    if f is not None:
        f.close()
    return data

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
            _create_directory_if_it_does_not_exist(dir_name)
            self.save()

    def save(self):
        """
        Saves the configuration options to a file.
        The attributes of this object.
        """
        exclude_list = ["_config_path"] # some attributes not to save
        data = {
            "configuration": {}, 
            "appname": configure.APPNAME, 
            "version": configure.VERSION
            }
        print("Saving config to %s" % (self._config_path))
        for key in sorted(self.__dict__.keys()): 
            value = self.__dict__[key]
            if key in exclude_list:
                pass #print("Excluding attribute %s since it is in the exclude list." % (key))
            else:
                data["configuration"][key] = value
        _save(self._config_path, data)

    def save_as(self, file_name):
        _former_file_name = self._config_path
        self._config_path = file_name
        self.save()
        self._config_path = _former_file_name

    def _load(self):
        print("Loading configuration from %s" % (self._config_path))
        data = _load(self._config_path)
        print(str(data))
        for k in data["configuration"].keys():
            if hasattr(self, k):
                cast = type(getattr(self, k)) # a little cast, to get rid of unicode which should be strings.
                setattr(self, k, cast(data["configuration"][k]))
            else:
                print("Found configuration key %s but it is not supported in this version of Scenic." % (k))

class AddressBook(object):
    """
    READING & WRITING ADDRESS BOOK FILE 
    """
    def __init__(self):
        self.current_contact_is_new = False
        self.contact_list = [] # list of dicts with keys "name", "address", "auto_accept", "port"
        self.selected = 0 # index of the selected contact
        self.file_name = os.path.expanduser("~/.scenic/contacts.json")
        self.load()
    
    def get_currently_selected_contact(self):
        """
        @rtype: dict or None
        """
        return self.contact_list[self.selected]

    def load(self):
        """
        Loads the data from the addressbook file and populate the "contact_list" and "selected" attributes.
        """
        if os.path.isfile(self.file_name):
            print("Loading addressbook.")
            data = _load(self.file_name)
            try:
                self.selected = data["selected"]
            except KeyError:
                self.selected = None
            except IndexError:
                self.selected = None
            self.contact_list = data["contact_list"]
        else:
            print("No addressbook found.")

    def save(self):
        data = {
            "selected": self.selected,
            "contact_list": self.contact_list
            }
        print "saving addressbook to %s" % (self.file_name)
        _save(self.file_name, data)
