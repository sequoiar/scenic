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
Configuration for streams. 

Starts both sender and receiver using twisted. 
Returns deferred instances to which you can register callbacks.

NOTES:
 * make sure jackd is running (you must habe 8 in/out)
 * make sure there is no milhouse running. (pstree -p)
"""
#TODO: rename to utils.conf.py and delete the old one.
import sys
from pprint import pformat
from miville.utils import log
from miville.utils import serialize
log = log.start("info", True, True, "streams.conf")

# constants
GLOB_STARTSWITH = "startswith"
GLOB_CONTAINS = "contains" 

# constants for configuration types
TYPE_ENUM = "enum" # one value out of a list of strings (using the value, not the index)
TYPE_STR = "str"
TYPE_INT = "int"
TYPE_BOOL = "bool"
TYPE_FLOAT = "float"

# enums, singletons
_types = {int:TYPE_INT, str:TYPE_STR, float:TYPE_FLOAT, bool:TYPE_BOOL}
_single_db = None # singleton Database global instance

def get_single_db():
    """
    Factory singleton Database instance. 
    Return Database instance.
    """
    global _single_db
    if _single_db is None:
        _single_db = Database()
    return _single_db

def path_glob(entries, path, method=GLOB_STARTSWITH):
    """
    Returns a subset of a dict, whose keys start with path.

    :param entries: Dict of name = value configuration entries
    :param path: String in the /osc/way
    :param method: Either GLOB_STARTSWITH or GLOB_CONTAINS
    """
    # TODO: whose keys contain or match a regex.
    res = {}
    for k in entries.iterkeys():
        if method == GLOB_STARTSWITH:
            if k.startswith(path):
                res[k] = entries[k]
        elif method == GLOB_CONTAINS:
            try:
                i = k.index(path) # throws a ValueError if not found.
                res[k] = entries[k]
            except ValueError:
                pass
    return res

def _get_type_for_typename(type_name):
    for k, v in _types.iteritems():
        if v == type_name:
            return k
    raise ConfError("No such type name: " + str(type_name))

class ConfError(Exception):
    """
    Any config error.
    """
    pass

class IdPool(object): 
    """
    Pool of unique ID numbers.
    ID are in the range [0, sys.maxint]

    A priority is given to small numbers. 
    This means that if you remove ID #0, the next one to be allocated
    will be 0, since it is the smallest available number.
    """
    def __init__(self, allocated=[]):
       self.allocated = set(allocated)
    
    def get_all(self):
        """
        Returns the list of all allocated ID.
        """
        return list(self.allocated)

    def allocate(self):
        """
        Returns a newly allocated ID.
        """
        # TODO: make faster right now
        # O(n*n)
        i = 0
        ret = None
        while i < sys.maxint:
            if not i in self.allocated:
                ret = i
                break
            else:
                i += 1
        if i == sys.maxint:
            raise ConfError("No more unique ID available !")
        self.allocated.add(ret)
        return ret

    def remove(self, id):
        """
        Remove an ID in the pool.
        """
        if id in self.allocated:
            self.allocated.remove(id)
        else:
            raise ConfError("ID %s is not allocated. Cannot remove it." % (id))

    def __str__(self):
        ret = "POOL OF ID:"
        ret += str(list(self.allocated))
        return ret


class Field(object): 
    """
    Configuration field.
    
    This is like a class, but for field entries.
    Entries would be instances of a Field.
    """
    def __init__(self, name="/default", default=None, desc=None, values=None):
        self.name = name
        self.type = None
        self.default = default
        self.values = values
        self.desc = desc
        if self.type is None:
            if self.values is not None:
                self.type = TYPE_ENUM
            else:
                try:
                    self.type = _types[type(default)]
                except KeyError, e:
                    raise ConfError("Invalid field type %s" % (e))        

    def __str__(self):
        return "FIELD:\n" + pformat(self.__dict__)

class Setting(object):
    """
    A Setting contains entries.
    """
    # TODO: preset settings over 10000
    def __init__(self, name="default", desc="", id=0, entries=None):
        self.entries = {} # dict of name => value (value can be one of the field types.)
        self.name = name # TODO: unicode
        self.desc = desc # TODO unicode
        self.id = id # int
        if entries is not None:
            self.entries.update(entries)

    def serialize(self):
        """ 
        Returns a dict with all members and values 
        """
        flattened = {'entries':self.entries, 'name':self.name, 'desc':self.desc, 'id':self.id}
        return flattened

    def unserialize(self, data):
        """ 
        :param data: dict with all members and values 
        """
        self.entries = data['entries']
        self.name = data['name']
        self.desc = data['desc']
        self.id = data['id']

    def __str__(self):
        ret = "SETTING:\n" + pformat(self.__dict__)
        return ret

class Set(object):
    """
    A Set is a set of ID numbers. 
    
    Id numbers are usually an option from which to choose from, such as a choice of setting id. 
    For example, one would choose from either the "8-channel raw", "8-channel vorbis" or "24-channel vorbis".
    The user cannot choose all three, of course. 

    Typically, the name of a set would be an ASCII string intended to be used by developers only.
    The desc field is where the description will be stored. That's what shown to the user.
    """
    def __init__(self, name="", desc=u"", ids=None):
        self.name = name
        self.desc = desc
        self.ids = set([])
        if ids is not None:
            for i in ids:
                self.ids.add(i)
    
    def add(self, id):
        """
        :param id: int
        """
        self.ids.add(id)
    
    def remove(self, id):
        """
        :param id: int
        """
        self.ids.remove(id)

    def get_all(self):
        """
        :rettype: list
        """
        return list(self.ids)

    def serialize(self):
        """ 
        Returns a dict with all members and values 
        :rettype: dict
        """
        flattened = {'ids':self.get_all(), 'name':self.name, 'desc':self.desc}
        return flattened
        
    def unserialize(self, data):
        """ 
        :param data: dict with all members and values 
        """
        self.name = data['name']
        self.desc = data['desc']
        self.ids = set(data['ids'])

class Profile(object): 
    """
    A Profile contains settings ID.
    """
    # FIXME: rename settings to setting_ids
    #TODO: Add detailled desc.
    # TODO: preset settings over 10000
    def __init__(self, name="default", id=None, desc=""):
        self.settings = [] # list of settings ID #TODO: rename to settings_ids
        self.name = name # TODO unicode
        self.id = id # int
        self.desc = desc # TODO unicode
    
    def serialize(self):
        """ 
        Returns a dict with all members and values 
        """
        flattened = {'settings':self.settings, 'name':self.name, 'id':self.id, 'desc':self.desc}
        return flattened

    def unserialize(self, data):
        """ 
        Returns a dict with all members and values 
        """
        self.settings = data['settings']
        self.name = data['name']
        self.id = data['id']
        self.desc = data['desc']

    def __str__(self):
        return "PROFILE:\n" + pformat(self.__dict__)

class Database(object): 
    """
    The database of config options.
    
    Contains the declarations of fields, and also the settings, which contains entries. 
    Settings can be gathered into profiles. 
    Settings and profiles have unique ID. (Using a IdPool)
    """
    def __init__(self):
        self.clear_all()

    def clear_all(self):
        """
        Clears the whole database from any setting, profile, field, etc.
        Use with caution.
        One might not want to clear fields while an application is running !
        """
        self.fields = {} # dict of name => Field 
        self.settings = {} # dict of id => Setting
        self.profiles = {} # dict of id => Profile
        self.sets = {} # dict of name => Set
        self.settings_pool = IdPool()
        self.profiles_pool = IdPool()
    
    def add_set(self, name, desc="", settings_ids=None):
        """
        Creates a set and adds it to the sets.
        :param name: str Name of the set (for developers)
        :param desc: str Description. (for end users)
        :param settings_ids: list of int. This options makes it easier to add many settings_ids to a set at creation time.
        :rettype: Set instance
        """
        if self.sets.has_key(name):
            raise ConfError("A set named \"%s\" already exists." % (name))
        s = Set(name, desc)
        self.sets[name] = s
        if settings_ids is not None:
            for setting_id in settings_ids:
                self.add_setting_to_set(name, setting_id)
        return s

    def get_set(self, name):
        """
        :param name: str
        :rettype: Set
        """
        if not self.sets.has_key(name):
            raise ConfError("There is no set named \"%s\" in the database." % (name))
        return self.sets[name]
    
    def add_setting_to_set(self, set_name, setting_id):
        """
        Adds a setting id to a set.
        :param set_name: str Name of a set.
        :param setting_id: int ID of a setting
        """
        s = self.get_set(set_name)
        setting = self.get_setting(setting_id)
        s.add(setting_id)
    
        
    def get_setting(self, setting_id):
        """
        Returns setting for setting ID.
        Raises ConfError if setting ID does not exists.
        """
        if not self.settings.has_key(setting_id):
            raise ConfError("No such setting: " + str(setting_id))
        else:
            setting  = self.settings[setting_id]
            return setting

    def get_profile(self, profile_id):
        """
        Returns profile for profile ID.
        Raises ConfError if profile ID does not exists.
        """
        if not self.profiles.has_key(profile_id):
            raise ConfError("No such profile: " + str(profile_id))
        else:
            return self.profiles[profile_id]

    def get_field(self, name):
        """
        Returns field for field name.
        Raises ConfError if field name does not exists.
        """
        if not self.fields.has_key(name):
            raise ConfError("No such field: " + str(name))
        else:
            return self.fields[name]

    def add_field(self, name, default=None, desc=None, values=None):
        """
        Adds a config field.
        """
        field = Field(name, default=default, desc=desc, values=values)
        self.fields[name] = field
        return field

    def remove_field(self, name):
        """
        Removes a config field.
        Raises a ConfError if the field is used in a setting.
        """
        # TODO: what should we do with the settings that use this field?
        field = self.get_field(name)
        for setting in self.settings.itervalues():
            if name in setting.entries.keys():
                raise ConfError("Cannot remove field \"%s\" since it is used by setting %d. (%s)" % (name, setting.id, setting.name))
        # else
        del self.fields[name]
        
    def add_setting(self, name=None, desc="", entries=None):
        """
        Adds a setting
        :param settings: dict Keys are field names, values are entry values.
        :param desc: str Description.
        :param name: str Field name.
        :return: Setting instance
        """
        if name is None:
            name = "" # why not...
        setting_id = self.settings_pool.allocate()
        setting = Setting(name, desc, id=setting_id)
        self.settings[setting_id] = setting
        if entries is not None:
            if type(entries) is not dict:
                raise ConfError("Entries must be a dict.")
            for name, value in entries.iteritems():
                self.add_entry(setting_id, name, value)
        return setting

    def add_profile(self, name="", desc="", settings=None):
        """
        Adds a profile
        :param settings: list
        :return: Profile instance
        """
        profile_id = self.profiles_pool.allocate()
        profile = Profile(name, id=profile_id, desc=desc)
        self.profiles[profile_id] = profile
        if settings is not None:
            for setting_id in settings:
                if setting_id not in profile.settings:
                    profile.settings.append(setting_id)
        return profile

    def duplicate_profile(self, profile_id, name=None):
        """
        Creates a new profile with all the same infos as the given profile ID.
        :param profile_id: int ID of the profile to duplicate.
        :rettype: Profile
        """
        profile = self.get_profile(profile_id)
        if name is None:
            name = "%s (copy)" % (profile.name)
        return self.add_profile(name, profile.desc, profile.settings)
    
    def add_setting_to_profile(self, profile_id, setting_id):
        """
        Adds a setting to a profile.
        The order of arguments matter ! 
        
        Raises ConfError if setting ID does not exists.
        Raises ConfError if profile ID does not exists.
        """
        log.debug("add_setting_to_profile(profile_id=%s, setting_id=%s)" % (profile_id, setting_id))
        setting = self.get_setting(setting_id)
        profile = self.get_profile(profile_id)
        if setting_id not in profile.settings:
            # check if that setting is part of a set, if so, make sure no other setting of that set
            # is in this profile
            for set_name, the_set in self.sets.iteritems():
                all_in_set = the_set.get_all()
                if setting_id in all_in_set:
                    log.debug("Setting %s is in set %s" % (setting.name, set_name))
                    for other_setting_id in all_in_set:
                        if other_setting_id in profile.settings:
                            other_setting_name = self.get_setting(other_setting_id)
                            log.info("Replacing setting %s in profile %s by setting %s since they are both in the %s set." % (other_setting_name, profile.name, setting.name, the_set.name))
                            self.remove_setting_from_profile(profile_id, other_setting_id)
            profile.settings.append(setting_id)
    
    def delete_setting(self, setting_id):
        """
        Deletes a setting.
        Raises ConfError if setting ID does not exists.
        """
        setting = self.get_setting(setting_id)
        for profile in self.profiles.itervalues():
            if setting.id in profile.settings:
                raise ConfError("Cannot remove setting %d (%s) since profile %d (%s) uses it" % (setting.id, setting.name, profile.id, profile.name)) 
        del self.settings[setting_id]
        del setting
        self.settings_pool.remove(setting_id)
 
    def delete_profile(self, profile_id):
        """
        Deletes a profile.
        Raises ConfError if profile ID does not exists.
        """
        # XXX
        profile = self.get_profile(profile_id)
        del self.profiles[profile_id]
        del profile
        self.profiles_pool.remove(profile_id)
    
    def remove_setting_from_profile(self, profile_id, setting_id):
        """
        Removes a setting from a profile.
        The order of arguments matter ! 

        Raises ConfError if setting ID does not exists.
        Raises ConfError if profile ID does not exists.
        """
        setting = self.get_setting(setting_id)
        profile = self.get_profile(profile_id)
        if setting_id in profile.settings:
            profile.settings.remove(setting_id)
        
    def add_entry(self, setting_id, field_name=None, value=None):
        """
        Adds an entry in a setting.
        Raises ConfError if setting ID does not exists.
        Raises ConfError if field name does not exists.
        Raises ConfError value has a bad type for the field type.
        Return None
        """
        global _types
        setting = self.get_setting(setting_id)
        field = self.get_field(field_name)
        try:
            if field.type == TYPE_ENUM:
                cast = str
            else:
                cast = _get_type_for_typename(field.type)
            value = cast(value)
        except TypeError, e:
            raise ConfError("""Bad value "%s" for field "%s". Should be of type "%s".""" % (value, field_name, field.type))
        setting.entries[field_name] = value
    
    def add_entries(self, setting_id, field_name_and_values={}):
        """
        Adds many entries at once to a setting.
        Return None
        """
        for field_name, value in field_name_and_values.iteritems():
            self.add_entry(setting_id, field_name, value)
    
    def get_entries_for_setting(self, setting_id):
        """
        Returns values for all fields in the setting.
        
        Returns a dict.
        Raises ConfError if setting ID does not exists.
        """
        setting = self.get_setting(setting_id)
        return setting.entries
    
    def get_entries_for_profile(self, profile_id):
        ret = {}
        profile = self.get_profile(profile_id)
        for setting_id in profile.settings: # list
            ret.update(self.get_entries_for_setting(setting_id))
        return ret

    def save_as_json(self, filename):
        """
        Saves data in the database as a JSON file.
        Does not save the fields.
        Unused so far 
        """
        log.debug("DATABASE DUMP : save_as_json")
        flattened = {}
        flattened['settings'] = {}
        for k, v in self.settings.iteritems():
            flattened['settings'][k] = v.serialize()
        flattened['profiles'] = {}
        for k, v in self.profiles.iteritems():
            flattened['profiles'][k] = v.serialize()
        flattened['settings_pool'] = list(self.settings_pool.allocated)
        flattened['profiles_pool'] = list(self.profiles_pool.allocated)
        
        flattened['sets'] = {}
        for k, v in self.sets.iteritems():
            flattened['sets'][k] = v.serialize()
        serialize.serialize_json(filename, flattened)

    def load_from_json(self, filename):
        """
        Gets member values from json 
        At startup, first populate the fields, next load from json.
        """
        log.debug("DATABASE LOAD")
        # get rid of any settings/profiles in this db
        # except fields !!
        fields = self.fields
        self.clear_all()
        self.fields = fields
        # get database data from file
        data = serialize.unserialize_json(filename)
        # json stores keys in unicode, so we need to convert int keys to int
        for k, v in data['settings'].iteritems():
            key = int(k)
            self.settings[key] = Setting()
            self.settings[key].unserialize(v)
            for entry in self.settings[key].entries:
                self.get_field(entry) # will raise if field does not exist
        for k, v in data['profiles'].iteritems():
            key = int(k)
            self.profiles[key] = Profile()
            self.profiles[key].unserialize(v)
            # make sure setting id exists
            for setting_id in self.profiles[key].settings:
                if setting_id not in self.settings.keys():
                    raise serialize.UnserializeError("Invalid setting id %" % (setting_id))
        for k, v in data["sets"].iteritems():
            self.sets[k] = Set()
            self.sets[k].unserialize(v)
        self.settings_pool = IdPool(data['settings_pool'])
        self.profiles_pool = IdPool(data['profiles_pool'])
