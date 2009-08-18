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
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.

"""
Module for configuration profile saving.

Inspired by GConf.

The programmer use the methods of the Client command usign twisted callbacks and errbacks.
The Database contains Fields and Profiles. A profile contains Entries. The entries are of the 
"type" of a Field. For instance, you would create an Entry called 
"/video/device/0/number" whose Field would be called "Video Device Number" with type \
"int" and a default value of 0. You would then create a "default" Profile, for example which 
contain a "/video/device/0/number" with 0 as a value. You would then duplicate this profile 
to a new one named "modified", for example. In that profile, you might want to choose an 
other value for the "/video/device/0/number", such as 1.

This way, you can manage different profiles of your configuration options. 
These can be used as presets, or edited by the user.

The IClient interface can be implemented using GConf later if needed, since the design of 
this module is inspired from GConf.

Installation::
  
  sudo apt-get install python-twisted 
"""
# TODO: create valid python code using repr() (and parsed by eval())
# TODO: call Client.file_save() and Client.file_load() periodically.
# TODO: change api for Client.profile_duplicate()
# TODO: change the syntax of the profile file so that it is pure python

import os
import pprint
import zope.interface
from twisted.internet import defer
from twisted.python import failure
# from twisted.spread import jelly

DEFAULT_FILE_NAME = "~/.ratsconf"
DEFAULT_PROFILE_NAME = "default"
# GLOBAL_PROFILE_NAME = "GLOBAL"
_single_database = None # singleton

class ConfError(Exception):
    """
    Any error that can occur within this module.
    """
    def __init__(self, message, method_called=None, **keywords):
        Exception.__init__(self, message)
        self.method_called = method_called
        self.keywords = keywords

class IClient(zope.interface.Interface):
    """
    Client to the Configuration server.
    
    Inspired by Gconf, using twisted for the MVC pattern.
    """
    # notifieds = zope.interface.Attribute("""List of callbacks to be notified when an attribute's value changes.""")
    def __init__(self):
        pass
    
    def field_add(self, name="default", type="str", default=None, desc=""):
        """  
        Adds a new field.
        Returns Deferred
        """
        pass
    def field_list(self):
        """
        Lists the fields. 
        Returns Deferred.
        """
        pass
    def field_remove(self, name):
        """ 
        Removes an entry in the current profile.
        returns Deferred 
        """
        pass

    def field_get(self, name):
        """
        Gets a field by its name.    
        returns Deferred 
        """
        pass

    def entry_get(self, field_name):
        """
        Gets an entry by its name.    
        returns Deferred 
        """
        pass

    def entry_set(self, field_name, value):
        """
        Sets the value of an entry in the currently selected profile.
        returns Deferred 
        """
        pass

    def entry_add(self, field_name, value):
        """
        Adds an entry in the current profile.
        returns Deferred
        """
        pass

    def entry_remove(self, field_name):
        """  
        Removes an entry from the current profile.
        returns Deferred 
        """
        pass

    def entry_default(self, field_name):
        """
        Sets an entry value to its default.
        returns Deferred 
        """
        pass
    
    def entry_list(self):
        """
        List all entries in the current profile.
        Returns Deferred.
        """
        pass

#     def notified_add(self, name, callback, *user_data):
#         """  returns  """
#         pass
# 
#     def notified_remove(self, name):
#         """  returns  """
#         pass

    def profile_add(self, name, desc=""):
        """  
        Creates a new empty profile.
        returns Deferred 
        
        Also selects the newly created profile.
        """
        pass

    def profile_save(self, name):
        """
        Saves the currently selected profile to the database.
        returns Deferred 
        """
        # TODO: does not save to file for now. Save it !
        pass

    def profile_load(self, name):
        """
        Selects a profile
        returns Deferred 
        """
        pass

    def profile_default(self) :
        """  
        Selects the default profile.
        returns Deferred 
        """
        pass
    
    def profile_set_default(self, name):
        """
        Changes the default profile.
        Does not switch to it, though.
        returns Deferred 
        """
        pass

    def profile_list(self):
        """
        List all profiles instances in the database.
        returns Deferred 
        """
        pass
    
    def profile_remove(self, name):
        """
        Remove a profile instance in the database.
        returns Deferred 
        Selects the default profile after removal. 
        It is not possible to remote the default profile.
        """
        pass

    def profile_duplicate(self, name, desc=""):
        """  
        Creates a new profile, duplicating the previously selected one.
        returns Deferred 
        
        Also selects the newly created profile.
        """
        pass

    def file_load(self):
        """
        Reads the config in a file. 
        """
        pass
    
    def file_save(self):
        """
        Saves the config to a file.
        """
        pass

class Success(object):
    """
    Simple successful result.
    """
    def __init__(self, value=None, method_called=None, **keywords):
        self.value = value
        self.method_called = method_called
        self.keywords = keywords

def _create_success(result=None, method_called=None, **keywords):
    """
    Returns a successful Deferred instance with a Success instance 
    as argument to its callbacks.
    """
    return defer.succeed(Success(result, method_called, **keywords))

def _create_failure(message_or_error=None, method_called=None, **keywords):
    """
    Returns a failed deferred instance.
    """
    error = None
    if isinstance(message_or_error, ConfError):
        error = message_or_error
        error.method_called = method_called
    else:
        error = ConfError(str(message_or_error), method_called, **keywords)
    return defer.fail(failure.Failure(error))

class Client(object):
    # TODO: rename to ConfClient ?
    zope.interface.implements(IClient)
    """
    Simple configuration client.
    """
    # return a Deferred object already called back with the value of result
    # return defer.succeed(result)
    # You can only call Deferred.callback or Deferred.errback once.
    def __init__(self):
        global _single_database
        global DEFAULT_PROFILE_NAME
        if _single_database is None:
            _single_database = Database()
        self.db = _single_database
        self.current_profile_name = DEFAULT_PROFILE_NAME
        self.notifieds = []
    
    def _get_profile(self, name=None):
        """
        :return: current profile instance or raise ConfError.
        No deferred here.
        """
        if name is None:
            name = self.current_profile_name
        try:
            profile = self.db.profiles[name]
            return profile
        except KeyError:
            try:
                name = self.db.default_profile_name
                profile = self.db.profiles[name]
                return profile
            except KeyError:
                raise ConfError("No such profile: %s" % (name))
    
    def _get_default_profile(self):
        try:
            return self.db.profiles[DEFAULT_PROFILE_NAME]
        except KeyError, e:
            raise ConfError("No such profile: %s" % (DEFAULT_PROFILE_NAME))
        
    def _get_entry(self, field_name):
        """
        Returns an entry from the current profile, identified by its name.
        No deferred here.

        May raise a ConfError
        """
        profile = self._get_profile()
        try:
            return profile.entries[field_name]
        except KeyError:
            profile = self._get_default_profile()
            try:
                return profile.entries[field_name]
            except KeyError:
                raise ConfError("No such entry: %s" % (name))

    def _get_field(self, name):
        """
        :return: current field instance or raise ConfError.
        No deferred here.
        """
        try:
            field = self.db.fields[name]
            return field
        except KeyError:
            raise ConfError("No such field: %s" % (name))
        
    def field_add(self, name="default", type="str", default=None, desc=""):
        """  
        Adds a new field.
        returns Deferred 
        """
        # TODO: put default after after type
        if self.db.fields.has_key(name):
            return _create_failure("Field %s already exists." % (name), "field_add")
        else:
            self.db.fields[name] = Field(name=name, default=default, type=type, desc=desc)
            return _create_success(name, "field_add")

    def field_list(self):
        """
        Lists fields. (as dict)
        Returns a Deferred.
        """
        # TODO: rename to plural (fields_list)
        return _create_success(self.db.fields, "field_list")

    def field_remove(self, name):
        """ 
        Removes an entry in the current profile.
        returns Deferred 
        """
        # TODO: check is any entry uses it.
        try:
            field = self._get_field(name)
            del self.db.fields[name]
            return _create_success(name, "field_remove")
        except ConfError, e:
            return _create_failure(e, "field_remove")

    def field_get(self, name):
        """
        Gets a field by its name.    
        returns Deferred 
        """
        try:
            ret = self._get_field(name)
            return _create_success("field_get", ret)
        except ConfError, e:
            return _create_failure(e, "field_get")

    def entry_add(self, field_name, value):
        """  
        Adds an entry in the current profile.
        returns Deferred 
        """
        # if self.db.fields.has_key(name):
        try:
            profile = self._get_profile() # current one (state machine)
            field = self._get_field(field_name) # check if field exists
            profile.entries[field_name] = Entry(field_name, value) # field_name
            return _create_success(field_name, "entry_add")
        except ConfError, e:
            return _create_failure(e, "entry_add")

    def entry_remove(self, name):
        """ 
        Removes an entry from the current profile.
        returns Deferred 
        """
        try:
            profile = self._get_profile()
            entry = self._get_entry(name)
            del profile.entries[name]
            return _create_success(name, "entry_remove")
        except ConfError, e:
            return _create_failure(e, "entry_remove")

    def entry_get(self, field_name):
        """
        Gets an entry value by its name.    
        returns Deferred 
        """
        # TODO: value, not the entry itself.
        try:
            ret = self._get_entry(field_name)
            return _create_success(ret, "entry_get")
        except ConfError, e:
            return _create_failure(e, "entry_get")
    
    def entry_list(self):
        try:
            profile = self._get_profile()
            entries = profile.entries
            return _create_success(entries, "entry_list")
        except ConfError, e:
            return _create_failure(e, "entry_list")
            

    def entry_set(self, name, value):
        """  
        Sets the value of an entry.
        returns Deferred 
        """
        try:
            entry = self._get_entry(name)
            #if type(value) is not type(entry.value):
            field = self._get_field(entry.field_name)
            cast = str # default
            if field.type == "str":
                cast = str
            elif field.type == "int":
                cast = int
            elif field.type == "float":
                cast = float # TODO: more casting types.
            entry.value = cast(value)
            return _create_success(name, "entry_set")
        except ValueError, e:
            return _create_failure("Wrong type %s of %s for entry %s." % (type(value), value, name))
        except ConfError, e:
            return _create_failure(e, "entry_set")

    def profile_add(self, name, desc=""):
        """  
        Creates a new empty profile.
        returns Deferred 
        
        Also selects the newly created profile.
        """
        if self.db.profiles.has_key(name):
            return _create_failure("Profile %s already exists." % (name), "profile_add")
        else:
            self.db.profiles[name] = Profile(name, desc)
            self.current_profile_name = name
            return _create_success(name, "profile_add")

    def profile_duplicate(self, name, desc=""):
        """  
        Creates a new profile, duplicating the previously selected one.
        returns Deferred 
        
        Also selects the newly created profile.
        """
        # TODO: duplicate current profile?
        # TODO: move current_profile_name to the Client ?
        # TODO: remove desc argument ?
        try:
            previous_profile = self.current_profile_name
            if self.db.profiles.has_key(name):
                return _create_failure("Profile %s already exists." % (name), "profile_new")
            else:
                self.db.profiles[name] = Profile(name, desc)
                self.current_profile_name = name
                try:
                    for entry in self._get_profile(previous_profile).entries.values():
                        self.db.profiles[name].entries[entry.field_name] = entry
                except ConfError, e:
                    return _create_failure(e, "profile_duplicate")
                return _create_success(name, "profile_new")
        except ConfError, e:
            return _create_failure(e, "profile_duplicate")

    def profile_save(self, name):
        """
        Saves the currently selected profile to the database.
        returns Deferred 
        """
        # Nothing to do for now.
        # TODO: save to file?
        return _create_success(True, "profile_save")

    def profile_load(self, name):
        """
        Selects a profile
        returns Deferred 
        """
        # TODO: rename profile_select?
        if self.db.profiles.has_key(name):
            self.current_profile_name = name
            return _create_success(name, "profile_load")
        else:
            return _create_failure("Not such profile: %s." % (name), "profile_load")

    def profile_default(self) :
        """  
        Selects the default profile.
        returns Deferred 
        """
        #TODO: rename to profile_load_default?
        name = self.db.default_profile_name
        return self.profile_load(name)
    
    def profile_set_default(self, name):
        """
        Changes the default profile.
        Does not switch to it, though.
        returns Deferred 
        """
        if self.db.profiles.has_key(name):
            self.db.default_profile_name = name
            return _create_success(name, "profile_set_default")
        else:
            return _create_failure("Not such profile: %s." % (name), "profile_set_default")

    def profile_list(self):
        """
        List all profiles instances in the database.
        returns Deferred 
        """
        profiles = self.db.profiles.values()
        return _create_success(profiles, "profile_list")
    
    def profile_remove(self, name):
        """
        Removes a profile instances from the database.
        returns Deferred 
        Selects the default profile after removal. 
        It is not possible to remote the default profile.
        """
        if name == DEFAULT_PROFILE_NAME:
            return _create_failure("It is not possible to remove the default profile.", "profile_remove")
        else:
            try:
                profile = self._get_profile(name)
                del self.db.profiles[name]
                return _create_success(name, "profile_remove")
            except KeyError, e:
                return _create_failure("Not such profile: %s." % (name), "profile_remove")
        
    def entry_default(self, name):
        """
        Sets an entry value to its default.
        returns Deferred 
        """
        try:
            entry = self._get_entry(name)
            field = self._get_field(entry.field_name)
            entry.value = field.default
            return _create_success(name, "entry_default")
        except ConfError, e:
            return _create_failure(e, "entry_default")

#    def notified_add(self, name, callback, *user_data):
#        """  returns  """
#        pass

#    def notified_remove(self, name):
#        """  returns  """
#        pass

    def file_save(self):
        """
        Saves conf to a file.
        """
        #TODO
        filename = self.db.file_name
        lines = []
        lines.append("# \n")
        lines.append("# This file has a custom format. See miville/utils/conf.py \n")
        lines.append("# The first word is the class of the object.\n")
        lines.append("# After the '|' character, the dict enumerates its attributes values.\n")
        lines.append("# Comment lines start with the '#' character.\n")
        lines.append("# SCHEMAS ----- \n")
        for field in self.db.fields.values():
            lines.append("%s | %s\n" % ("Field", {"name":field.name, "default":field.default, "type":field.type, "desc":field.desc}))
        lines.append("# PROFILES ----- \n")
        for profile in self.db.profiles.values():
            lines.append("# ----- \n")
            lines.append("%s | %s\n" % ("Profile", {"name":profile.name, "desc":profile.desc}))
            for entry in profile.entries.values():
                lines.append("%s | %s\n" % ("Entry", {"field_name":entry.field_name, "value":entry.value}))
        try:
            f = open(filename, 'w')
            f.writelines(lines)
            f.close()
        except IOError, e:
            raise ConfError(str(e))
        except OSError, e:
            raise ConfError(str(e))

    def file_load(self):
        """
        Loads conf from a file.
        """
        filename = self.db.file_name
        fields = {}
        profiles = {}
        last_profile = None
        default_profile_name = None
        try:
            try:
                f = open(filename, 'r')
                lines = f.readlines()
                f.close()
            except IOError, e:
                raise ConfError(str(e))
            except OSError, e:
                raise ConfError(str(e))
            for line in lines:
                if not line.startswith("#"):
                    nameval = line.split("|")
                    class_name = nameval[0].strip()
                    data = nameval[1].strip()
                    if class_name == "Field":
                        kwargs = eval(data) # dict
                        field = Field(**kwargs)
                        fields[field.name] = field
                    elif class_name == "Profile":
                        kwargs = eval(data) # dict
                        profile = Profile(**kwargs)
                        profiles[profile.name] = profile
                        last_profile = profile.name
                    elif class_name == "Entry":
                        kwargs = eval(data) # dict
                        if last_profile is None:
                            raise ConfError("No profile to put that entry in." + str(line))
                        else:
                            profile = profiles[last_profile]
                            entry = Entry(**kwargs)
                            profile.entries[entry.field_name] = entry
                    elif class_name == "default_profile_name":
                        default_profile_name = data # str
            self.db.profiles = profiles
            self.db.fields = fields
            self.db.default_profile_name = default_profile_name
        except ConfError, e:
            _create_failure(e, "file_load")
        except SyntaxError, e:
            _create_failure("Error parsing config file : %s" % (str(e)))

class Database(object):
    """
    Database for the simple client.
    Contains profiles of entries
    """
    def __init__(self, file_name=None, default_profile_name=None, profiles={}, fields={}):
        global DEFAULT_FILE_NAME
        if file_name is None:
            file_name = os.path.expanduser(DEFAULT_FILE_NAME)
        if default_profile_name is None:
            default_profile_name = DEFAULT_PROFILE_NAME
        self.file_name = file_name
        self.default_profile_name = default_profile_name 
        self.profiles = profiles # name: instance pairs
        self.fields = fields # name: instance pairs
        if len(self.profiles) == 0:
            self.profiles[DEFAULT_PROFILE_NAME] = Profile(DEFAULT_PROFILE_NAME, "Default profile.")
            # self.profiles[GLOBAL_PROFILE_NAME] = Profile(GLOBAL_PROFILE_NAME, "Global profile.")
        
    def __str__(self):
        text = "DB with fields:\n%s\nProfiles:\n%s\n" % (self.profiles, self.fields)
        text += "current_profile:%s\nDefault profile:%s\n" % (self.current_profile, self.default_profile_name)
        return text

class ChangedNotification(object):
    def __init__(self, method_called=None, new_value=None, old_value=None, name=None) :
        self.method_called = method_called
        self.new_value = new_value
        self.old_value = old_value
        self.name = name

class Profile(object):
    """
    Snapshot of the set of profiles.
    """
    def __init__(self, name, desc="") :
        self.name = name
        self.desc = desc
        self.entries = {} # field_names:instance 
    
    def __str__(self):
        return "Profile: %s (%s).\nEntries: %s\n" % (str(self.entries))

class Entry(object):
    """
    One config option value.
    """
    def __init__(self, field_name, value): #, field_name=None):
        #self.name = name
        self.field_name = field_name 
        self.value = value

    def __str__(self):
        return "Entry %s: %s" % (self.field_name, self.value)
        #, self.field_name)

class Field(object):
    """
    Kind of entry.
    """
    def __init__(self, name="default", default=None, type="str", desc=""):
        self.name = name # 
        self.type = type # 
        self.default = default # 
        self.desc = desc # 
        if self.default is None:
            if self.type == "int":
                self.default = 0
            elif self.type == "float":
                self.default = 0.
            elif self.type == "str":
                self.default = ""
        # self.writable = True # 
        # self._owner = None # 

    def __str__(self):
        return "Field %s (%s) (%s) with default=%s." % (self.name, self.type, self.desc, self.default)

if __name__ == "__main__":
    def print_entries(result):
        entries = result.value
        print("Entries: ------------")
        for entry in entries.values():
            print(str(entry))
        print("-------------")
        
    client = Client()
    # default profile --------------------
    client.field_add("/foo/bar/0/size", type="int", default=1, desc="Foo bar")
    client.field_add("/foo/bar/1/size", type="int", default=1, desc="Foo bar")
    client.entry_add("/foo/bar/0/size", 2)
    client.entry_add("/foo/bar/1/size", 2)
    client.entry_list().addCallback(print_entries)
    # no need to run reactor since everything is synchronous so far.
    client.entry_list().addCallback(print_entries)
    client.profile_duplicate("other") # --------------------------
    client.entry_default("/foo/bar/1/size")
    client.entry_add("/foo/bar/0/size", 2)
    client.entry_list().addCallback(print_entries)
    #print("JELLY SAVE TO: %s" % (client.db.file_name))
    #serial_save(client.db.file_name, client.db)
    #restored_db = serial_load(client.db.file_name)
    #print(db)
    print("Saving to file...")
    client.file_save()
    print("Loading from file..")
    client.file_load()
    client.entry_list().addCallback(print_entries)

