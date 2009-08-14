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

    def entry_get(self, key):
        """ returns Deferred """
        pass

    def entry_set(self, key, value):
        """  returns Deferred """
        pass

    def entry_add(self, key, value, schema_name, writable):
        """  returns Deferred """
        pass

    def entry_remove(self, key):
        """  returns Deferred """
        pass

    def entry_default(self, key):
        """  returns Deferred """
        pass

    def notified_add(self, key, callback, *user_data):
        """  returns  """
        pass

    def notified_remove(self, key):
        """  returns  """
        pass

    def state_new(self, name):
        """  returns Deferred """
        pass

    def state_save(self, name):
        """  returns Deferred """
        pass

    def state_load(self, name):
        """  returns Deferred """
        pass

    def state_default(self) :
        """  returns Deferred """
        pass
    
    def state_set_default(self, name):
        """  returns Deferred """
        pass

    def state_list(self):
        """  returns Deferred """
        pass

    def state_add(self, state):
        """  returns Deferred """
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

DEFAULT_FILE_NAME = "~/.ratsconf"
DEFAULT_STATE_NAME = "default"
_single_database = None # singleton

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
        if _single_database is None:
            _single_database = Database()
        self.db = _single_database
        self.notifieds = []
    
    def _get_state(self, name=None):
        """
        :return: current state instance or raise ConfError.
        No deferred here.
        """
        if name is None:
            name = self.db.default_state_name
        try:
            state = self.db.states[name]
            return state
        except KeyError:
            raise ConfError("No such state: %s" % (name))

    def _get_entry(self, key):
        """
        Returns and entry from the current state, identified by its key.
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
        
    def schema_add(self, name="default", default="default", type=str, desc=""):
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
        Removes an entry in the current state.
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
            entry.value = schema.type(value)
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
            self.db.current_state_name = name
            return _create_success(name, "state_new")

    def state_duplicate(self, name, desc=""):
        """  
        Creates a new state, duplicating the previously selected one.
        returns Deferred 
        
        Also selects the newly created state.
        """
        # TODO: duplicate current state?
        # TODO: move current_state_name to the Client ?
        previous_state = self.db.current_state_name
        if self.db.states.has_key(name):
            return _create_failure("State %s already exists." % (name), "state_new")
        else:
            self.db.states[name] = State(name, desc)
            self.db.current_state_name = name
            try:
                for entry in self._get_state(previous_state).entries.values():
                    self.db.states[name].entries[entry.key] = entry
            except ConfError, e:
                return _create_failure(e, "state_duplicate")
            
                # TODO
            return _create_success(name, "state_new")

    def state_save(self, name):
        """  returns Deferred """
        # Nothing to do for now.
        # TODO: save to file?
        return _create_success(True, "state_save")

    def state_load(self, name):
        """
        Selects a state
        returns Deferred 
        """
        if self.db.states.has_key(name):
            self.db.current_state_name = name
            return _create_success(name, "state_load")
        else:
            return _create_failure("Not such state: %s." % (name), "state_load")

    def state_default(self) :
        """  returns Deferred """
        name = self.db.default_state_name
        if self.db.states.has_key(name):
            self.db.current_state_name = name
            return _create_success(name, "state_default")
        else:
            return _create_failure("Not such state: %s." % (name), "state_default")
    
    def state_set_default(self, name):
        """  returns Deferred """
        if self.db.states.has_key(name):
            self.db.default_state_name = name
            return _create_success(name, "state_set_default")
        else:
            return _create_failure("Not such state: %s." % (name), "state_set_default")

    def state_list(self):
        """  returns Deferred """
        states = self.db.states.values()
        return _create_success(states, "state_list")
    
    def entry_default(self, key):
        """  returns Deferred """
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

    def serial_save(self, filename, obj):
        """
        Saves any python data type to a file.

        Might throw an SerializeError
        """
        #TODO
        li = jelly.jelly(obj)
        try:
            f = open(filename, 'w')
            f.write(pprint.pformat(li))
            f.close()
        except IOError, e:
            raise SerializeError(e.message)
        except OSError, e:
            raise SerializeError(e.message)

    def serial_load(self, filename):
        """
        Loads any python data type from a file.

        Might throw an UnserializeError
        """
        try:
            f = open(filename, 'r')
            li = eval(f.read()) # do this only on trusted data !
            f.close()
        except IOError, e:
            raise UnserializeError(e.message)
        except OSError, e:
            raise UnserializeError(e.message)
        try:
            obj = jelly.unjelly(li)
        except jelly.InsecureJelly, e:
            raise UnserializeError(e.message)
        except AttributeError, e:
            raise UnserializeError(e.message)
        else:
            return obj
class Database(object):
    """
    Database for the simple client.
    Contains states of entries
    """
    def __init__(self, file_name=None):
        global DEFAULT_FILE_NAME
        global DEFAULT_STATE_NAME
        if file_name is None:
            file_name = os.path.expanduser(DEFAULT_FILE_NAME)
        self.file_name = file_name
        self.default_state_name = DEFAULT_STATE_NAME
        self.current_state_name = self.default_state_name
        self.states = {} # name: instance pairs
        self.schemas = {} # name: instance pairs
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
    def __init__(self, name="default", default="value", type=str, desc=""):
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
    client.schema_add("foo_bar_size", default=1, type=int, desc="Foo bar")
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
    
