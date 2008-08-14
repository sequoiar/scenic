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
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.


# System imports
import os, os.path
#try:
#    import cPickle as pickle
#except ImportError:
#    import pickle
import re

# Twisted imports
from twisted.spread.jelly import jelly, unjelly

# App imports
from utils import log
from utils.i18n import to_utf

log = log.start('info', 1, 0, 'adb')



class AddressBook(object):
    """ """
    
    def __init__(self, filename):
        filename = to_utf(filename)
        self.contacts = {'_selected':None}
        self.filename = os.environ['HOME'] + '/.' + filename + '/' + filename + '.adb'
        self.read()
        
    def add(self, name, address):
        name = to_utf(name)
        if name in self.contacts:
            return False
        self.contacts[name] = Contact(name, address)
        self.write()
        return True
        
    def remove(self, name):
        name = to_utf(name)
        if name not in self.contacts:
            return False
        del self.contacts[name]
        self.write()
        return True
    
    def modify(self, name, new_name, address):
        name = to_utf(name)
        new_name = to_utf(new_name)
        if name in self.contacts:
            del self.contacts[name]
            self.contacts[new_name] = Contact(new_name, address)
            self.write()
            return True
        return False       

    def select(self, name):
        name = to_utf(name)
        if name in self.contacts:
            self.contacts['_selected'] = name
            self.write()
            return True
        return False

    def read(self):
        try:
            adb_file = open(self.filename, 'r')
        except:
            log.warning('Address Book file %s not found.' % self.filename)
        else:
            self.contacts = unjelly(eval(adb_file.read().replace('\n', '').replace('\t', '')))
#            self.contacts = pickle.load(adb_file)
            adb_file.close()

    def write(self):
        if len(self.contacts) > 1:
            directory = os.path.dirname(self.filename)
            if not os.path.isdir(directory):
                try:
                    os.makedirs(directory)
                except:
                    log.warning('Could not create the directory %s.' % directory)
                    return
            try:
                adb_file = open(self.filename, 'w')
            except:
                log.warning('Could not open the Address Book file %s.' % self.filename)
            else:
                dump = repr(jelly(self.contacts))
                level = 0
                nice_dump = []
                for c in dump:
                    if c == '[':
                        if level > 0:
                            nice_dump.append('\n' + '\t'*level)
                        level += 1
                    elif c == ']':
                        level -= 1
                    nice_dump.append(c)
                adb_file.write(''.join(nice_dump))
#                pickle.dump(self.contacts, adb_file, 1)
                adb_file.close()

    
class Contact(object):
    """Class representing a contact in the address book"""
    
    def __init__(self, name, address):
        self.name = name
        self.address = address
        
        self.port = None
    
    def type(self):
        address = self.address
        if isinstance(address, list):
            return 'group'
        if isinstance(address, str):
            # could be a SIP type address
            if '@' in address:
                if re.match('^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,4}$', address, re.I):
                    return 'sip_name'
                if re.match('^[A-Z0-9._%+-]+@\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$', address, re.I):
                    adr = address.split('@')
                    values = [int(i) for i in adr[1].split('.')]
                    if self._ip_range(values):
                        return 'sip_ip'
                return None
            # could be an IP type address
            elif re.match('^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$', address):
                # verify the range of each number
                values = [int(i) for i in address.split('.')]
                if self._ip_range(values):
                    # verify if the first number is in the multicast range
                    if values[0] >= 224 and values[0] <= 239:
                        return 'multicast'
                    return 'ip'
        return None
        
    def _ip_range(self, nums):
        for num in nums:
            if num < 0 or num > 255:
                return False
            return True
        
                
                
if __name__ == '__main__':
    adb = AddressBook('test')
    adb.add('Gaspésie', '111.23.111.32')
    adb.add('Simon', 'ppa.sat.qc.ca')
    adb.add('SIP contact', 'longjohn@monami.com')
    adb.modify('Simon', 'Simonp', 'allo.sat.qc.ca')
    adb.remove('SIP contact')
    adb.select('Simonp')
