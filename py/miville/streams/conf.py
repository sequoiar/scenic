#!/usr/bin/env python
# -*- coding: utf-8 -*-
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
import os
from pprint import pformat
# from twisted.spread import jelly # TODO
#from miville.utils import serialize
from miville.utils import log
# TODO: use log more and more.
log = log.start("info", 1, 0, "streams.conf")

# constants
GLOB_STARTSWITH = "startswith"
GLOB_CONTAINS = "contains" 

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


class ContactInfos(object):
    """
    Informations regarding the remote and local IP, etc. 
    """
    #TODO: use comchan to get local IP.
    #TODO: set ou local port
    DEFAULT_PORT = 2222 # com_chan port (TODO: or use api.get_config()....
    def __init__(self, **kwargs):
        # default value to be overriden
        self.remote_addr = "127.0.0.1"
        self.local_addr = "127.0.0.1"
        self.remote_port = self.DEFAULT_PORT
        self.local_port = self.DEFAULT_PORT 
        self.contact = None # miville.addressbook.Contact object.
        self.__dict__.update(kwargs)

    def set_remote_contact(self, contact):
        """
        Configure these contact infos to match the infos 
        of the addressbook.Contact object.
        """
        self.contact = contact
        self.remote_addr = contact.address
        self.remote_port = contact.port
        if self.remote_port is None:
            self.remote_port = self.DEFAULT_PORT

    def get_id(self):
        """
        Returns a unique identified that streams can use as a key to store
        their contact.
        For now, since the contact name changes, and this class is meant to be 
        created and deleted a lot, let's use adde:port as a key.
        """
        if self.remote_port is None:
            self.remote_port = self.DEFAULT_PORT
        return "%s:%s" % (self.remote_addr, self.remote_port)

    def get_contact_name(self):
        if self.contact is not None:
            return self.contact.name
        else:
            return "Unknown"

# constants for configuration types
TYPE_ENUM = "enum" # one value out of a list of strings (using the value, not the index)
TYPE_STR = "str"
TYPE_INT = "int"
TYPE_BOOL = "bool"
TYPE_FLOAT = "float"

_types = {int:TYPE_INT, str:TYPE_STR, float:TYPE_FLOAT, bool:TYPE_BOOL}

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

# class Entry(object):
#     """
#     One entry is like an instance of a Field.
#     """
#     def __init__(self, field_name="/default", value=None):
#         self.field_name = field_name # str
#         self.value = value # any simple python type

class Setting(object):
    """
    A Setting contains entries.
    """
    # TODO: preset settings over 10000
    def __init__(self, name="default", desc="", id=0): #id=None, desc="", entries={}):
        self.entries = {} # dict of name => value (value can be one of the field types.)
        self.name = name # TODO: unicode
        self.desc = desc # TODO unicode
        self.id = id # int

    def __str__(self):
        ret = "SETTING:\n" + pformat(self.__dict__)
        return ret

# TODO:
#class MutuallyExclusiveProfilesList(object):
#    def __init__(self):
#        self.profiles = []
#    def add_profile(self, id):
#        self.profiles.append(id)

class Profile(object): 
    """
    A Profile contains settings ID.
    """
    #TODO: Add detailled desc.
    # TODO: preset settings over 10000
    def __init__(self, name="default", id=None, desc=""):
        self.settings = [] # list of settings ID
        self.name = name # TODO unicode
        self.desc = desc # TODO unicode
        self.id = id # int

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
        self.fields = {} # dict of name => Field 
        self.settings = {} # dict of id => Setting
        self.profiles = {} # dict of id => Profile
        self.pool = IdPool([]) # IdPool instance
    
    def get_setting(self, setting_id):
        """
        Returns profile for profile ID.
        Raises ConfError if profile ID does not exists.
        """
        if not self.settings.has_key(setting_id):
            raise ConfError("No such setting: " + str(setting_id))
        else:
            setting  = self.settings[setting_id]
            # print("get_setting() : " + str(setting))
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
        Throws a ConfError if the field is used in a setting.
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
        setting_id = self.pool.allocate()
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
        profile_id = self.pool.allocate()
        profile = Profile(name, id=profile_id, desc=desc)
        self.profiles[profile_id] = profile
        if settings is not None:
            for setting_id in settings:
                if setting_id not in profile.settings:
                    profile.settings.append(setting_id)
        return profile
    
    def add_setting_to_profile(self, profile_id, setting_id):
        """
        Adds a setting to a profile.
        The order of arguments matter ! 
        
        Raises ConfError if setting ID does not exists.
        Raises ConfError if profile ID does not exists.
        """
        setting = self.get_setting(setting_id)
        profile = self.get_profile(profile_id)
        if setting_id not in profile.settings:
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
        #XXX: used to be  with default values, overriden by entries in the specified setting ID.
        #    setting = self.get_setting(setting_id)
        #    ret = {}
        #    for field_name, value in setting.entries.iteritems():
        #        if field_name not in self.fields.keys():
        #            raise ConfError("Field name \"%s\" does not exist but setting %d (%s) has a entry with this name." % (field_name, setting.id, setting.name))
        #        else:
        #            ret[field_name] = value
        #        #if setting.entries.has_key(field_name):
        #        #    # ret[field_name] = setting.entries[field_name].value
        #        #    ret[field_name] = setting.entries[field_name]
        #        #else:
        #        #    ret[field_name] = self.fields[field_name].default
        #    return ret
        setting = self.get_setting(setting_id)
        return setting.entries
    
    def get_entries_for_profile(self, profile_id):
        ret = {}
        profile = self.get_profile(profile_id)
        for setting_id in profile.settings: # list
            ret.update(self.get_entries_for_setting(setting_id))
        return ret

    def __str__(self):
        ret = "DATABASE DUMP"
        ret += "Fields:\n"
        for k, v in self.fields.iteritems():
            ret += "    %s : %s\n" % (k, v)
        ret += "Settings:\n"
        for k, v in self.settings.iteritems():
            ret += "    %s : %s\n" % (k, v)
        ret += "Profiles:\n"
        for k, v in self.profiles.iteritems():
            ret += "    %s : %s\n" % (k, v)
        ret += "Pool:\n"
        ret += str(self.pool.__dict__)
        return ret
