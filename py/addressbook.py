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

"""
This module deals with the address book and its contacts.
"""

# System imports
try:
    import cPickle as pickle
except ImportError:
    import pickle
import re
from types import UnicodeType
import copy

# Twisted imports
from twisted.spread.jelly import jelly, unjelly

# App imports
from miville.utils import log
from miville.utils import common
from miville.utils.i18n import to_utf
from miville.errors import AddressBookError, AddressBookNameError
from miville.connectors.states import DISCONNECTED, CONNECTED 

log = log.start('debug', 1, 0, 'adb')


class AddressBook(object):
    """
    Class representing the Address Book of the application.
    The Address Book is written to disk an read back.
    The Address Book is mostly a dictionnary of Contact instances.
    
    Once a contact is selected, the user can start the communication with her 
    using a basic connector. THe next step is to start the streams. 
    """
    def __init__(self, filename, api=None):
        """
        Filename must be given. The Address Book is save in file named
        'filename.adb' in the directory '~/.filename/'.
        """
        self.api = api
        self.filename = common.install_dir(filename)
        self.major = 1
        self.minor = None
        self.dup_suffix = '_copy'
        self.contacts = {}
        self.selected = None
        self.read()
        Contact.adb = self

    def add(self, name, address, port=None, auto_created=False, auto_answer=False, connector=None, setting=0):
        """
        Adds a contact to the Address Book.
        
        Name and address are mandatory, port is optional and if connector is
        None, it will be deduce from the address type. Setting is set to 0
        (base setting) by default.

        Address, port and setting are validated and exception are raise
        on problems.
        """
        name = to_utf(name)
        if name in self.contacts:
            raise AddressBookNameError, 'Name %s already in Address Book' % name
        self.contacts[name] = Contact(name, address, port, auto_created, auto_answer, connector, setting)
        log.debug("adding contact %s %s" % (name, address))
        self.write()
        return True

    def delete(self, name=None):
        """
        Deletes the named contact from the Address Book or the selected
        if no name is given.
        """
        name = self._get_name(name)
        del self.contacts[name]
        if self.selected == name:
            self.selected = None   #TODO: Could add a 'select first/next one'
        self.write()
        return True

    def modify(self, name=None, new_name=None, address=None, port=None, auto_answer=None, setting=None, connector=None):
        """
        Changes one or more attributes of a contact.
        If no name is given, modify the selected contact.
        """
        name = self._get_name(name)
        contact = self.contacts[name]

        if new_name is None and setting is None and address is None and port is None:
            raise AddressBookError, 'No property to change.'

        new_name = to_utf(new_name)
        if new_name and new_name != name:
            if new_name in self.contacts:
                raise AddressBookNameError, 'The name "%s" already exist.' % new_name
            contact.name = new_name
            del self.contacts[name]
            self.contacts[new_name] = contact
            if self.selected == name:
                self.selected = new_name

        contact.set_port(port)
        contact.set_address(address)
        contact.assign_connector(connector)
        if auto_answer != None:
            contact.auto_answer = auto_answer
        contact.setting = setting
        
        self.write()
        return True

    def duplicate(self, name=None, new_name=None):
        """
        Adds a copy of the named contact in the Address Book and add the string
        ' (copy)' to is name if no new name is given, else give the new name.
        If no name is given, copy the selected contact.
        """
        name = self._get_name(name)
        new_name = to_utf(new_name)
        if new_name in self.contacts:
            raise AddressBookNameError, 'This name %s already exist.' % new_name
        if new_name:
            name_copy = new_name
        else:
            ver = 1
            while True:
                if ver > 1:
                    suffix = '%s %s' % (self.dup_suffix, ver)
                else:
                    suffix = self.dup_suffix
                name_copy = '%s%s' % (name, suffix)
                if name_copy not in self.contacts:
                    break
                ver += 1
        self.contacts[name_copy] = copy.copy(self.contacts[name])
        self.contacts[name_copy].name = name_copy

        self.write()
        return True

    def select(self, name):
        """
        Selects one contact in the Address Book (by name). It become the 'active'
        contact.
        """
        name = to_utf(name)
        if name not in self.contacts:
            raise AddressBookError, 'Name %s not in contacts' % name
        self.selected = name

        self.write()
        return True

    def get_current(self):
        """
        Return the selected contact or return None if none is selected.
        """
        if self.selected in self.contacts:
            return self.contacts[self.selected]
        else:
            return None

    def get_contact(self, name=None):
        """
        Return the named contact instance or raise an Exception if it doesn't
        exist.
        """
        name = self._get_name(name)
        return self.contacts[name]

    def find_contact(self, address, port=None, connector=None):
        """
        Find a contact by its address and port.
        """
        for contact in self.contacts.values():
            if contact.address == address \
            and (connector == None or connector == contact.connector):
                if contact.port == None:
                    if port == None:
                        return contact
                    if connector == None and port == self.api.get_default_port(contact.connector):
                        return contact
                    if connector != None and port == self.api.get_default_port(connector):
                        return contact
                else:
                    if contact.port == port or port == None:
                        return contact
        return None

    def client_contact(self, address, port=None):
        """
        Create a temporary contact for the caller if he is not already in the
        Address Book. Contact are searched by address and port (optional).
        """
        contact = self.find_contact(address, port)
        if not contact:
            suffix = '(auto)'
            if port:
                name = '%s:%s_%s' % (address, port, suffix)
            else:
                name = '%s %s' % (address, suffix)
            self.add(name, address, port, True)
            contact = self.contacts[name]
        # select the contact
        self.selected = contact.name
        return contact

    def save_client_contact(self, name=None, new_name=None, auto_answer=False):
        """
        Saved permanently an auto created contact to the Address Book.
        If no name is given, the selected is use. If new_name is given the
        contact will be saved under this new name.
        """
        contact = self.get_contact(name)
        if not contact.auto_created:
            raise AddressBookError, 'This contact \'%s\' is already saved.' % contact.name
        contact.auto_created = False
        if new_name != None or auto_answer == True:
            self.modify(contact.name, new_name, auto_answer=auto_answer)
        self.write()

    def _get_name(self, name):
        """
        If name is None return the selected contact name if there is.
        Return the utf-8 version of the name if name is given and is
        in the contact list.
        """
        if name == None:
            if self.get_current():
                name = self.selected
            else:
                raise AddressBookError, 'No contact selected.'
        else:
            name = to_utf(name)
        if name not in self.contacts:
            raise AddressBookError, 'Name %s not in contacts.' % name
        return name

    def read(self):
        """
        Read the Address Book file (.adb) from the disk.
        """
        try:
            adb_file = open(self.filename, 'r')
            log.info("Opening address book file %s in read mode." % (self.filename))
        except IOError:
            log.warning('Address Book file %s not found.' % self.filename)
        else:
            serialized = False
            version = re.findall('v(\d+)\.(\d+)', adb_file.readline())
            if version:
                self.major = int(version[0][0])
                self.minor = int(version[0][1])

                if self.major == 1:
                    try:
                        self.contacts = unjelly(eval(adb_file.read().replace('\n', '').replace('\t', '')))
                    except:
                        log.error('Unable to read the Address Book file %s. Bad format.' % self.filename)
                    else:
                        serialized = True
            else:
                adb_file.seek(0)
                try:
                    self.contacts = pickle.load(adb_file)
                except:
                    log.error('Unable to read the Address Book file %s. Bad format.' % self.filename)
                else:
                    serialized = True
            if serialized:
                # get the selected contact and remove it from the contacts dict
                self.selected = self.contacts['_selected']
                del self.contacts['_selected']
                # add the connection attributes to all the contacts
                # and set the state to DISCONNECTD (change this in the future
                # to support recovery)
                for contact in self.contacts.values():
                    contact.connection = None
                    contact.state = DISCONNECTED
            adb_file.close()

    def write(self, state=True):
        """
        Write back to disk the Address Book. Raise error on problems.
        """
        if self.contacts:
            try:
                adb_file = open(self.filename, 'w')
                log.info("Opening address book file %s in write mode." % (self.filename))
            except:
                log.error('Could not open the Address Book file %s in write mode.' % self.filename)
                raise AddressBookError, 'Could not open the Address Book file %s.' % self.filename
            else:
                # make a copy of contacts dict to remove the connection attribute
                # without affecting the current contacts
                contacts = {}
                for name, contact in self.contacts.items():
                    if state or not contact.auto_created:
                        contacts[name] = copy.copy(contact)
                        if hasattr(contacts[name], 'connection'):
                            contacts[name].connection = None
                            del contacts[name].connection
                # add the selected contact to the dict
                contacts['_selected'] = self.selected
                if self.major == 1:
                    dump = repr(jelly(contacts))
                    level = 0
                    nice_dump = ['v1.0\n']
                    for char in dump:
                        if char == '[':
                            if level > 0:
                                nice_dump.append('\n' + '\t' * level)
                            level += 1
                        elif char == ']':
                            level -= 1
                        nice_dump.append(char)
                    adb_file.write(''.join(nice_dump))
                else:
                    pickle.dump(contacts, adb_file, 1)
                adb_file.close()
                
            # notify the observers that the addressbook as change
            self.api.get_contacts(self)


class Contact(object):
    """
    Class representing a contact in the address book
    """
    adb = None

    def __init__(self, name, address, port=None, auto_created=False, auto_answer=False, connector=None, setting=0):
        """
        Name and address are mandatory, port is optional and if connector is
        None, it will be deduce from the address type. Setting is set to 0
        (base setting) by default.

        Name, address, port and setting are validated and exception are raise
        on problems.

        """
        if len(name) < 1:
            raise AddressBookError, 'Name could not be empty.'
        self.name = name
        self.address = None
        self.port = None
        self.kind = None
        self.connector = None
        self.state = DISCONNECTED
        self.auto_created = auto_created
        self.auto_answer = auto_answer
        self.connection = None

        self.set_address(address)
        self.set_port(port)
        if connector:
            self.assign_connector(connector)

        if not isinstance(setting, int):    #TODO: validate if the setting exist
            raise AddressBookError, "The 'setting' arguments should be an integer. Got %s of type %s." % (setting, type(setting))
        self.setting = setting

    def __setattr__(self, name, value):
        """
        Catch any attribute changes and call Addressbook.write() (and api.notify())
        on change.
        """
        if name == 'setting':
            if value != None:
                value = int(value)
                
        write = False
        if name == 'state' and self.adb:
            state = getattr(self, 'state', None)
            if state != value and state is not None:
                write = True
                log.debug('Write on state change: %s - %s - %s' % (self.state, value, self.name))
        object.__setattr__(self, name, value)
        if write:
            self.adb.write()
        

    def set_address(self, address):
        """
        Set the address attribute of the contact in function of is kind
        (group or others).
        """
        if address != None:
            self.set_kind(address)
            if self.kind == 'group':
                self.address = address
            else:
                self.address = address.encode('utf-8')

# Not necessary, set the attribute directly, check __setattr__ above for more detail    
#    def set_setting(self, setting):
#        """
#        Sets the setting attribute. 
#        """
#        if setting != None:
#            id = int(setting)
#            self.setting = id
            
    def set_port(self, port):
        """
        Validate and set the port of the contact. If the port did not validate,
        an exception is raised. Port should be in between 1024 and 65535.
        The port given could be an integer, a string or a unicode string.
        If the port given is an empty string or the int 0, the port is set to None.
        """
        if port != None:
            if isinstance(port, str) or isinstance(port, UnicodeType):
                if port.isdigit():
                    port = int(port)
                elif port == '':
                    port = 0
                else:
                    raise AddressBookError, 'Invalid port format. (%s)' % port
            elif not isinstance(port, int):
                raise AddressBookError, 'Invalid port format. (%s)' % port
            if port == 0:
                self.port = None
                return
            if port < 1024 or port > 65535:
                raise AddressBookError, 'Port %s out of range. Should be in between 1024 and 65535.' % port
            self.port = port

    def set_kind(self, address):
        """
        Validate the given address, find is kind then set it.

        Kinds:
         :group:     a list of contact names
         :sip_name:  name@www.domain.org
         :sip_ip:    name@123.123.123.123
         :ip:        123.123.123.123
         :multicast: [224-239].123.123.123

        If the address did not validate an exception is raise.
        """
        # keep it to see if there's a change, if yes update connector
        prev_kind = self.kind

        kind = None
        if isinstance(address, UnicodeType):
            address = address.encode('utf-8')
        if isinstance(address, list):
            kind = 'group'  #TODO: Could add a verification if all names in the list are valid
        elif isinstance(address, str):
            # could be a SIP type address
            if '@' in address:
                if re.match('^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,4}$', address, re.I):
                    kind = 'sip_name'
                elif re.match('^[A-Z0-9._%+-]+@\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$', address, re.I) \
                and ip_range(address.split('@')[1]):
                    kind = 'sip_ip'
            # could be an IP type address
            # verify the range of each number
            elif re.match('^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$', address) \
            and ip_range(address):
                # verify if the first number is in the multicast range
                if 224 <= int(address.split('.')[0]) <= 239:
                    kind = 'multicast'
                else:
                    kind = 'ip'
        if not kind:
            raise AddressBookError, '%s is an invalid address type' % address

        self.kind = kind
        if self.kind != prev_kind:
            self.assign_connector()

    def assign_connector(self, connector=None):
        """
        Set the connector use by this contact. Connector should be a string.
        If none is given, one is deduce from the address kind.
        Matching rules can be change in the future.

        :param connector: Should be a valid connector
        :type connector: string or None
        """
        if connector:   #TODO: Validate if the connector is one of those loaded in the system
            self.connector = connector
        elif self.connector == None:
            if self.kind[0:3] == 'sip':
                self.connector = 'sip'
            elif self.kind != 'group':
                self.connector = 'basic'


def ip_range(address):
    """
    Validate that each number of an IP address is between 0 and 255.
    The address should be a string of the format '123.123.123.123'.
    """
    nums = [int(i) for i in address.split('.')]
    for num in nums:
        if num < 0 or num > 255:
            raise AddressBookError, 'Invalid IP range: %s' % num
    return True
