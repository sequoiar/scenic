#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof. If not, see <http:#www.gnu.org/licenses/>.


# System imports
import os, os.path
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
from utils import log
from utils.i18n import to_utf
from errors import AddressBookError

log = log.start('info', 1, 0, 'adb')


class AddressBook(object):
    """
    Class representing the Address Book of the application.
    The Address Book is writen to disk an read it back.
    The Address Book is mostly a dictionnary of Contact instances.
    """
    
    def __init__(self, filename):
        """
        Filename must be given. The Address Book is save in file named
        'filename.adb' in the directory '~/.filename/'.
        """
        filename = to_utf(filename).lstrip()
        if not filename:
            raise AddressBookError, 'File name <%s> is not valid.' % filename
        self.major = 1
        self.minor = None
        self.dup_suffix = ' (copy)'
        self.contacts = {'_selected':None}
        self.filename = os.environ['HOME'] + '/.' + filename + '/' + filename + '.adb'
        self.read()
            
    def add(self, name, address, port=None, connector=None, setting=0):
        """
        Add a contact to the Address Book.
        Name and address are mandatory, port is optional and if connector is
        None, it will be deduce from the address type. Setting is set to 0
        (base setting) by default.
        
        Address, port and setting are validated and exception are raise
        on problems.
        """
#        if kind(address):
        name = to_utf(name)
        if name in self.contacts:
            raise AddressBookError, 'Name %s already in contacts' % name
        self.contacts[name] = Contact(name, address, port, connector, setting)
        self.write()
        return True
        
    def delete(self, name=None):
        """
        Delete the named contact from the Address Book or the selected
        if no name is given.
        """
        name = self._get_name(name)
        del self.contacts[name]
        if self.contacts['_selected'] == name:
            self.contacts['_selected'] = None   #TODO: Could add a 'select first/next one'
        self.write()
        return True
    
    def modify(self, name=None, new_name=None, address=None, port=None):
        """
        Change one or more attributes of a contact.
        If no name is given, modify the selected contact.
        """
        name = self._get_name(name)
        contact = self.contacts[name]
        
        if new_name is None and address is None and port is None:
            raise AddressBookError, 'No property to change.'
        
        if new_name:
            new_name = to_utf(new_name)
            if new_name in self.contacts:
                raise AddressBookError, 'This name %s already exist.' % new_name
            contact.name = new_name
            del self.contacts[name]
            self.contacts[new_name] = contact
            if self.contacts['_selected'] == name:
                self.contacts['_selected'] = new_name
           
        contact.set_port(port)
        
        contact.set_address(address)
        
        self.write()
        return True

    def duplicate(self, name=None, new_name=None):
        """
        Add a copy of the named contact in the Address Book and add the string
        ' (copy)' to is name if no new name is given, else give the new name.
        If no name is given, copy the selected contact.
        """
        name = self._get_name(name)
        new_name = to_utf(new_name)
        if new_name in self.contacts:
            raise AddressBookError, 'This name %s already exist.' % new_name
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
        Select one contact in the Address Book (by name). It become the 'active'
        contact.
        """
        name = to_utf(name)
        if name not in self.contacts:
            raise AddressBookError, 'Name %s not in contacts' % name
        self.contacts['_selected'] = name
        self.write()
        return True
    
    def get_current(self):
        """
        Return the selected contact or return None if none is selected.
        """
        name = self.contacts['_selected']
        if name in self.contacts:
            return self.contacts[name]
        else:
            return None

    def _get_name(self, name):
        """
        If name is None return the selected contact name if there is.
        Return the utf-8 version of the name if name is given and is
        in the contact list.
        """
        if name == None:
            if self.get_current():
                name = self.contacts['_selected']
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
        except IOError:
            log.info('Address Book file %s not found.' % self.filename)
        else:
            version = re.findall('v(\d+)\.(\d+)', adb_file.readline())
            if version:
                self.major = int(version[0][0])
                self.minor = int(version[0][1])
                
                if self.major == 1:
                    self.contacts = unjelly(eval(adb_file.read().replace('\n', '').replace('\t', '')))
            else:
                adb_file.seek(0)
                try:
                    self.contacts = pickle.load(adb_file)
                except:
                    log.warning('Unable to read the Address Book file %s. Bad format.' % self.filename)
            adb_file.close()

    def write(self):
        """
        Write back to disk the Address Book. Raise error on problems.
        """
        if len(self.contacts) > 1:
            directory = os.path.dirname(self.filename)  #TODO: move this the Main/Startup
            if not os.path.isdir(directory):
                try:
                    os.makedirs(directory)
                except:
                    log.warning('Could not create the directory %s.' % directory)
                    raise AddressBookError, 'Could not create the directory %s.' % directory
            try:
                adb_file = open(self.filename, 'w')
            except:
                log.warning('Could not open the Address Book file %s.' % self.filename)
                raise AddressBookError, 'Could not open the Address Book file %s.' % self.filename
            else:
                if self.major == 1:
                    dump = repr(jelly(self.contacts))
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
                    pickle.dump(self.contacts, adb_file, 1)
                adb_file.close()

    
class Contact(object):
    """Class representing a contact in the address book"""
    
    def __init__(self, name, address, port=None, connector=None, setting=0):
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

        self.set_address(address)
        self.set_port(port)
        self.assign_connector(connector)
        
        if not isinstance(setting, int):    #TODO: validate if the setting exist
            raise AddressBookError, "The 'setting' arguments should be an integer. Got %s of type %s." % (setting, type(setting))
        self.setting = setting
                
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
            - group:       a list of contact names
            - sip_name:    name@www.domain.org
            - sip_ip:      name@123.123.123.123
            - ip:          123.123.123.123
            - multicast    [224-239].123.123.123
            
        If the address did not validate an exception is raise.
        """
#        return kind(self.address)
        if isinstance(address, list):
            self.kind = 'group'  #TODO: Could add a verification if all names in the list are valid
            return
        if isinstance(address, UnicodeType):
            address = address.encode('utf-8')
        if isinstance(address, str):
            # could be a SIP type address
            if '@' in address:
                if re.match('^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,4}$', address, re.I):
                    self.kind = 'sip_name'
                    return 
                elif re.match('^[A-Z0-9._%+-]+@\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$', address, re.I) \
                and ip_range(address.split('@')[1]):
                    self.kind = 'sip_ip'
                    return
                else:
                    raise AddressBookError, '%s is an invalid address type' % address
            # could be an IP type address
            # verify the range of each number
            elif re.match('^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$', address) \
            and ip_range(address):
                # verify if the first number is in the multicast range
                if 224 <= int(address.split('.')[0]) <= 239:
                    self.kind = 'multicast'
                    return
                self.kind = 'ip'
                return
        raise AddressBookError, '%s is an invalid address type' % address
    
    def assign_connector(self, connector=None):
        """
        Set the connector use by this contact. Connector should be a string.
        If none is given, one is deduce from the kind of address.
        Matching rules can be change in the future.
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
