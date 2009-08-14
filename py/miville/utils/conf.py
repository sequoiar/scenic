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
Module for configuration state saving.

Inspired by GConf.

The programmer use the methods of the Client command usign twisted callbacks and errbacks.
The Database contains Schemas and States. A state contains Entries. The entries are of the 
"type" of a Schema. For instance, you would create an Entry called 
"/video/device/0/number" whose Schema would be called "Video Device Number" with type \
"int" and a default value of 0. You would then create a "default" State, for example which 
contain a "/video/device/0/number" with 0 as a value. You would then duplicate this state 
to a new one named "modified", for example. In that state, you might want to choose an 
other value for the "/video/device/0/number", such as 1.

This way, you can manage different states of your configuration options. 
These can be used as presets, or edited by the user.

The IClient interface can be implemented using GConf later if needed, since the design of 
this module is inspired from GConf.

Installation::
  
  sudo apt-get install python-twisted 
"""
# TODO: call Client.file_save() and Client.file_load() periodically.
# TODO: change api for Client.state_duplicate()
# TODO: change the syntax of the state file so that it is pure python

import os
import pprint
from zope.interface import implements 
from zope.interface import Interface
from zope.interface import Attribute
from twisted.internet.defer import Deferred
from twisted.internet.defer import succeed
from twisted.internet.defer import fail
from twisted.python.failure import Failure
# from twisted.spread import jelly

DEFAULT_FILE_NAME = "~/.ratsconf"
DEFAULT_STATE_NAME = "default"
_single_database = None # singleton

class ConfError(Exception):
    """
    Any error that can occur within this module.
    """
    def __init__(self, message, method_called=None) :
        Exception.__init__(self, message)
        self.method_called = method_called


class IClient(Interface):
    """
    Client to the Configuration server.
    
    Inspired by Gconf, using twisted for the MVC pattern.
    """
    notifieds = Attribute("""List of callbacks to be notified when an attribute's value changes.""")
    def __init__(self):
        pass
    
    def schema_add(self, name="default", default="default", type="str", desc=""):
        """  
        Adds a new schema.
        Returns Deferred
        """
        pass
    
    def schema_remove(self, name):
        """ 
        Removes an entry in the current state.
        returns Deferred 
        """
        pass

    def schema_get(self, name):
        """
        Gets a schema by its name.    
        returns Deferred 
        """
        pass

    def entry_get(self, key):
        """
        Gets an entry by its key.    
        returns Deferred 
        """
        pass

    def entry_set(self, key, value):
        """
        Sets the value of an entry in the currently selected state.
        returns Deferred 
        """
        pass

    def entry_add(self, key, value, schema_name):
        """
        Adds an entry in the current state.
        returns Deferred
        """
        pass

    def entry_remove(self, key):
        """  
        Removes an entry from the current state.
        returns Deferred 
        """
        pass

    def entry_default(self, key):
        """
        Sets an entry value to its default.
        returns Deferred 
        """
        pass
    
    def entry_list(self):
        """
        List all entries in the current state.
        Returns Deferred.
        """
        pass

    def notified_add(self, key, callback, *user_data):
        """  returns  """
        pass

    def notified_remove(self, key):
        """  returns  """
        pass

    def state_add(self, name, desc=""):
        """  
        Creates a new empty state.
        returns Deferred 
        
        Also selects the newly created state.
        """
        pass

    def state_save(self, name):
        """
        Saves the currently selected state to the database.
        returns Deferred 
        """
        # TODO: does not save to file for now. Save it !
        pass

    def state_load(self, name):
        """
        Selects a state
        returns Deferred 
        """
        pass

    def state_default(self) :
        """  
        Selects the default state.
        returns Deferred 
        """
        pass
    
    def state_set_default(self, name):
        """
        Changes the default state.
        Does not switch to it, though.
        returns Deferred 
        """
        pass

    def state_list(self):
        """
        List all states instances in the database.
        returns Deferred 
        """
        pass
    
    def state_duplicate(self, name, desc=""):
        """  
        Creates a new state, duplicating the previously selected one.
        returns Deferred 
        
        Also selects the newly created state.
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
    def __init__(self, value=None, method_called=None, *user_args):
        self.value = value
        self.method_called = method_called
        self.user_args = user_args

def _create_success(result=None, method_called=None):
    """
    Returns a successful Deferred instance with a Success instance 
    as argument to its callbacks.
    """
    return succeed(Success(result, method_called))

def _create_failure(message_or_error=None, method_called=None):
    """
    Returns a failed deferred instance.
    """
    error = None
    if isinstance(message_or_error, ConfError):
        error = message_or_error
        error.method_called = method_called
    else:
        error = ConfError(str(message_or_error), method_called)
    return fail(Failure(error))


class Client(object):
    implements(IClient)
    """
    Simple configuration client that uses twisted's Jelly for its backend.
    """
    # return a Deferred object already called back with the value of result
    # return defer.succeed(result)
    # You can only call Deferred.callback or Deferred.errback once.
    def __init__(self):
        global _single_database
        global DEFAULT_STATE_NAME
        if _single_database is None:
            _single_database = Database()
        self.db = _single_database
        self.current_state_name = DEFAULT_STATE_NAME
        self.notifieds = []
    
    def _get_state(self, name=None):
        """
        :return: current state instance or raise ConfError.
        No deferred here.
        """
        if name is None:
            name = self.current_state_name
        try:
            state = self.db.states[name]
            return state
        except KeyError:
            try:
                name = self.db.default_state_name
                state = self.db.states[name]
                return state
            except KeyError:
                raise ConfError("No such state: %s" % (name))

    def _get_entry(self, key):
        """
        Returns an entry from the current state, identified by its key.
        No deferred here.
        """
        state = self._get_state()
        try:
            entry = state.entries[key]
            return entry
        except KeyError:
            raise ConfError("No such entry: %s" % (key))

    def _get_schema(self, name):
        """
        :return: current schema instance or raise ConfError.
        No deferred here.
        """
        try:
            schema = self.db.schemas[name]
            return schema
        except KeyError:
            raise ConfError("No such schema: %s" % (name))
        
    def schema_add(self, name="default", default="default", type="str", desc=""):
        """  
        Adds a new schema.
        returns Deferred 
        """
        if self.db.schemas.has_key(name):
            return _create_failure("Schema %s already exists." % (name), "schema_add")
        else:
            self.db.schemas[name] = Schema(name, default, type, desc)
            return _create_success(name, "schema_add")

    def schema_remove(self, name):
        """ 
        Removes an entry in the current state.
        returns Deferred 
        """
        # TODO: check is any entry uses it.
        try:
            schema = self._get_schema(name)
            del self.db.schema[name]
            return _create_success(key, "schema_remove")
        except ConfError, e:
            return _create_failure(e, "schema_remove")

    def schema_get(self, name):
        """
        Gets a schema by its name.    
        returns Deferred 
        """
        try:
            ret = self._get_schema(name)
            return _create_success("schema_get", ret)
        except ConfError, e:
            return _create_failure(e, "schema_get")

    def entry_add(self, key, value, schema_name):
        """  
        Adds an entry in the current state.
        returns Deferred 
        """
        try:
            state = self._get_state()
            schema = self._get_schema(schema_name) # check if schema exists
            state.entries[key] = Entry(key, value, schema_name)
            return _create_success(key, "entry_add")
        except ConfError, e:
            return _create_failure(e, "entry_add")

    def entry_remove(self, key):
        """ 
        Removes an entry from the current state.
        returns Deferred 
        """
        try:
            state = self._get_state()
            entry = self._get_entry(key)
            del state.entries[key]
            return _create_success(key, "entry_remove")
        except ConfError, e:
            return _create_failure(e, "entry_remove")

    def entry_get(self, key):
        """
        Gets an entry by its key.    
        returns Deferred 
        """
        try:
            ret = self._get_entry(key)
            return _create_success(ret, "entry_get")
        except ConfError, e:
            return _create_failure(e, "entry_get")
    
    def entry_list(self):
        try:
            state = self._get_state()
            entries = state.entries
            return _create_success(entries, "entry_list")
        except ConfError, e:
            return _create_failure(e, "entry_list")
            

    def entry_set(self, key, value):
        """  
        Sets the value of an entry.
        returns Deferred 
        """
        try:
            entry = self._get_entry(key)
            #if type(value) is not type(entry.value):
            schema = self._get_schema(entry.schema_name)
            cast = str # default
            if schema.type == "str":
                cast = str
            elif schema.type == "int":
                cast = int
            elif schema.type == "float":
                cast = float # TODO: more casting types.
            entry.value = cast(value)
            return _create_success(key, "entry_set")
        except ValueError, e:
            return _create_failure("Wrong type %s of %s for entry %s." % (type(value), value, key))
        except ConfError, e:
            return _create_failure(e, "entry_set")

    def state_add(self, name, desc=""):
        """  
        Creates a new empty state.
        returns Deferred 
        
        Also selects the newly created state.
        """
        if self.db.has_key(name):
            return _create_failure("State %s already exists." % (name), "state_new")
        else:
            self.db.states[name] = State(name, desc)
            self.current_state_name = name
            return _create_success(name, "state_new")

    def state_duplicate(self, name, desc=""):
        """  
        Creates a new state, duplicating the previously selected one.
        returns Deferred 
        
        Also selects the newly created state.
        """
        # TODO: duplicate current state?
        # TODO: move current_state_name to the Client ?
        # TODO: remove desc argument ?
        try:
            previous_state = self.current_state_name
            if self.db.states.has_key(name):
                return _create_failure("State %s already exists." % (name), "state_new")
            else:
                self.db.states[name] = State(name, desc)
                self.current_state_name = name
                try:
                    for entry in self._get_state(previous_state).entries.values():
                        self.db.states[name].entries[entry.key] = entry
                except ConfError, e:
                    return _create_failure(e, "state_duplicate")
                return _create_success(name, "state_new")
        except ConfError, e:
            return _create_failure(e, "state_duplicate")

    def state_save(self, name):
        """
        Saves the currently selected state to the database.
        returns Deferred 
        """
        # Nothing to do for now.
        # TODO: save to file?
        return _create_success(True, "state_save")

    def state_load(self, name):
        """
        Selects a state
        returns Deferred 
        """
        if self.db.states.has_key(name):
            self.current_state_name = name
            return _create_success(name, "state_load")
        else:
            return _create_failure("Not such state: %s." % (name), "state_load")

    def state_default(self) :
        """  
        Selects the default state.
        returns Deferred 
        """
        name = self.db.default_state_name
        if self.db.states.has_key(name):
            self.current_state_name = name
            return _create_success(name, "state_default")
        else:
            return _create_failure("Not such state: %s." % (name), "state_default")
    
    def state_set_default(self, name):
        """
        Changes the default state.
        Does not switch to it, though.
        returns Deferred 
        """
        if self.db.states.has_key(name):
            self.db.default_state_name = name
            return _create_success(name, "state_set_default")
        else:
            return _create_failure("Not such state: %s." % (name), "state_set_default")

    def state_list(self):
        """
        List all states instances in the database.
        returns Deferred 
        """
        states = self.db.states.values()
        return _create_success(states, "state_list")
    
    def entry_default(self, key):
        """
        Sets an entry value to its default.
        returns Deferred 
        """
        try:
            entry = self._get_entry(key)
            schema = self._get_schema(entry.schema_name)
            entry.value = schema.default
            return _create_success(key, "entry_default")
        except ConfError, e:
            return _create_failure(e, "entry_default")

#    def notified_add(self, key, callback, *user_data):
#        """  returns  """
#        pass

#    def notified_remove(self, key):
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
        for schema in self.db.schemas.values():
            lines.append("%s | %s\n" % ("Schema", {"name":schema.name, "default":schema.default, "type":schema.type, "desc":schema.desc}))
        lines.append("# STATES ----- \n")
        for state in self.db.states.values():
            lines.append("# ----- \n")
            lines.append("%s | %s\n" % ("State", {"name":state.name, "desc":state.desc}))
            for entry in state.entries.values():
                lines.append("%s | %s\n" % ("Entry", {"key":entry.key, "value":entry.value, "schema_name":entry.schema_name}))
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
        schemas = {}
        states = {}
        last_state = None
        default_state_name = None
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
                    keyval = line.split("|")
                    class_name = keyval[0].strip()
                    data = keyval[1].strip()
                    if class_name == "Schema":
                        kwargs = eval(data) # dict
                        schema = Schema(**kwargs)
                        schemas[schema.name] = schema
                    elif class_name == "State":
                        kwargs = eval(data) # dict
                        state = State(**kwargs)
                        states[state.name] = state
                        last_state = state.name
                    elif class_name == "Entry":
                        kwargs = eval(data) # dict
                        if last_state is None:
                            raise ConfError("No state to put that entry in." + str(line))
                        else:
                            state = states[last_state]
                            entry = Entry(**kwargs)
                            state.entries[entry.key] = entry
                    elif class_name == "default_state_name":
                        default_state_name = data # str
            self.db.states = states
            self.db.schemas = schemas
            self.db.default_state_name = default_state_name
        except ConfError, e:
            _create_failure(e, "file_load")
        except SyntaxError, e:
            _create_failure("Error parsing config file : %s" % (str(e)))

class Database(object):
    """
    Database for the simple client.
    Contains states of entries
    """
    def __init__(self, file_name=None, default_state_name=None, states={}, schemas={}):
        global DEFAULT_FILE_NAME
        if file_name is None:
            file_name = os.path.expanduser(DEFAULT_FILE_NAME)
        if default_state_name is None:
            default_state_name = DEFAULT_STATE_NAME
        self.file_name = file_name
        self.default_state_name = default_state_name 
        self.states = states # name: instance pairs
        self.schemas = schemas # name: instance pairs
        if len(self.states) == 0:
            self.states[DEFAULT_STATE_NAME] = State(DEFAULT_STATE_NAME, "Default state.")
        
    def __str__(self):
        text = "DB with schemas:\n%s\nStates:\n%s\n" % (self.states, self.schemas)
        text += "current_state:%s\nDefault state:%s\n" % (self.current_state, self.default_state_name)
        return text

class ChangedNotification(object):
    def __init__(self, method_called=None, new_value=None, old_value=None, key=None) :
        self.method_called = method_called
        self.new_value = new_value
        self.old_value = old_value
        self.key = key

class State(object):
    """
    Snapshot of the set of states.
    """
    def __init__(self, name, desc="") :
        self.name = name
        self.desc = desc
        self.entries = {} # key:instance 
    
    def __str__(self):
        return "State: %s (%s).\nEntries: %s\n" % (str(self.entries))

class Entry(object):
    """
    One config option value.
    """
    def __init__(self, key, value, schema_name=None):
        self.key = key
        self.value = value
        self.schema_name = schema_name 

    def __str__(self):
        return "Entry %s: %s (%s)" % (self.key, self.value, self.schema_name)

class Schema(object):
    """
    Kind of entry.
    """
    def __init__(self, name="default", default="value", type="str", desc=""):
        self.name = name # 
        self.default = default # 
        self.type = type # 
        self.desc = desc # 
        # self.writable = True # 
        # self._owner = None # 
    def __str__(self):
        return "Schema %s (%s) (%s) with default=%s." % (self.name, self.type, self.desc, self.default)

if __name__ == "__main__":
    def print_entries(result):
        entries = result.value
        print("Entries: ------------")
        for entry in entries.values():
            print(str(entry))
        print("-------------")
        
    client = Client()
    client.schema_add("foo_bar_size", default=1, type="int", desc="Foo bar")
    client.entry_add("/foo/bar/0/size", 2, "foo_bar_size")
    client.entry_add("/foo/bar/1/size", 2, "foo_bar_size")
    client.entry_list().addCallback(print_entries)
    # no need to run reactor since everything is synchronous so far.
    client.entry_default("/foo/bar/1/size")
    client.entry_list().addCallback(print_entries)
    client.state_duplicate("other")
    #print("JELLY SAVE TO: %s" % (client.db.file_name))
    #serial_save(client.db.file_name, client.db)
    #restored_db = serial_load(client.db.file_name)
    #print(db)
    print("Saving to file...")
    client.file_save()
    print("Loading from file..")
    client.file_load()
    client.entry_list().addCallback(print_entries)

