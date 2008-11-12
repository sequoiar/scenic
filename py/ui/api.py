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

import sys

# App imports
from errors import * 


class ControllerApi(object):
    
    def __init__(self, notify):
        self.notify = notify
    
    def _start(self, core):
        self.core = core
        self.adb = core.adb
        self.all_streams = core.curr_setting.streams
        self.curr_streams = 'send'
        self.streams = self.all_streams[self.curr_streams]
        self.connectors = core.connectors
        self.connection = None


    ### Contacts ###
        
    def get_contacts(self, caller):
        self.notify(caller, self.adb.contacts)
        
    def add_contact(self, caller, name, address, port=None):
        try:
            result = self.adb.add(name, address, port)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)
        
    def delete_contact(self, caller, name=None):
        try:
            result = self.adb.delete(name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)
        
    def modify_contact(self, caller, name=None, new_name=None, address=None, port=None):
        try:
            result = self.adb.modify(name, new_name, address, port)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)
        
    def duplicate_contact(self, caller, name=None, new_name=None):
        try:
            result = self.adb.duplicate(name, new_name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)
        
    def select_contact(self, caller, name):
        try:
            result = self.adb.select(name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)
        
        
    ### Streams ###
    
    def start_streams(self, caller, address, channel=None):
        self.notify(caller, self.streams.start(address, channel))

    def stop_streams(self, caller):
        self.notify(caller, self.streams.stop())

    def set_streams(self, caller, attr, value):
        self.notify(caller, self.streams.set_attr(attr, value))
        
    def select_streams(self, caller, name):
        if name in self.all_streams:
            self.streams = self.all_streams[name]
            self.curr_streams = name
            self.notify(caller, (name, True))
        else:
            self.notify(caller, (name, False))

    def list_streams(self, caller):
        self.notify(caller, (self.all_streams, self.curr_streams))


    ### Stream ###
    
    def set_stream(self, caller, name, kind, attr, value):
        stream = self.streams.get(name, kind)
        if stream:
            self.notify(caller, (stream.set_attr(attr, value), name), kind + '_set')
        else:
            self.notify(caller, name, 'not_found')

    def settings_stream(self, caller, name, kind):
        self.notify(caller, (self.streams.get(name, kind), name), kind + '_settings')
        
    def add_stream(self, caller, name, kind, engine):
        self.notify(caller, (self.streams.add(name, kind, engine, self.core), name), kind + '_add')
        
    def delete_stream(self, caller, name, kind):
        self.notify(caller, (self.streams.delete(name, kind), name), kind + '_delete')
        
    def rename_stream(self, caller, name, new_name, kind):
        self.notify(caller, (self.streams.rename(name, new_name, kind), name, new_name), kind + '_rename')
        
    def list_stream(self, caller, kind):
        self.notify(caller, self.streams.list(kind), kind + '_list')
        
    
    ### Connect ###
    
    def start_connection(self, caller):
        contact = self.adb.get_current()
        connector = self.connectors[contact.kind()]
        client = connector.connect(self, contact.address, contact.port)
        self.notify(caller, ('Trying to connect with %s (%s)...' % (contact.name, contact.address), client))
        
    def stop_connection(self, caller):
        self.stop_streams(caller)
        if self.curr_streams == 'send':
            contact = self.adb.get_current()
            connector = self.connectors[contact.kind()]
            client = connector.disconnect(self, contact.address, contact.port)
        else:
            connector = self.connectors[self.connection[2]]
            client = connector.disconnect(self, self.connection[0], self.connection[1])
        self.notify(caller, 'Communication was stopped.', 'info')

    def accept_connection(self, caller, client):
        client.accept()
        self.notify(caller, 'Begining to receive...', 'info')

    def refuse_connection(self, caller, client):
        client.refuse()
        self.notify(caller, 'You refuse the connection.', 'info')
    
    def set_connection(self, address, port, connector):
        self.connection = (address, port, connector)




                