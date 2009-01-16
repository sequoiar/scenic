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

""" Public API of Sropulpof """

import sys

# App imports
from errors import *
import connectors
from connectors.states import *
import devices

class ControllerApi(object):
    """
    The controller API that all controllers (such as cli.CliController) must use.
    """

    def __init__(self, notify):
        self.notify = notify

    def _start(self, core):
        """
        Starts the API once all parts have been loaded.
        """
        self.core = core
        self.adb = core.adb
        self.all_streams = core.curr_setting.streams
        self.curr_streams = 'send'
        self.streams = self.all_streams[self.curr_streams]
        self.connectors = core.connectors
        self.connection = None
        devices.start(self) # api as an argument

    ### Contacts ###

    def get_contacts(self, caller):
        """
        Get the list of all the contacts.

        :rtype: a tuple of 2 items:

            - a dictionnary of Contact instances
            - the name of the selected Contact
        """
        self.notify(caller, (self.adb.contacts, self.adb.selected))

    def get_contact(self, name=None):
        """
        Get the named contact instance.
        """
        return self.adbself.connectors.get_contact(name)

    def find_contact(self, address, port=None, connector=None):
#        return self.adb.find_contact(address, port)
        """
        Find a contact by its address and port.
        """
        for contact in self.adb.contacts.values():
            if contact.address == address \
            and (connector == None or connector == contact.connector):
                if contact.port == None:
                    if port == None:
                        return contact
                    if connector == None and port == self.get_connector_port(contact.connector):
                        return contact
                    if connector != None and port == self.get_connector_port(connector):
                        return contact
                else:
                    if contact.port == port or port == None:
                        return contact
        return None

    def get_connector_port(self, connector):
        return self.connectors[connector].PORT

    def client_contact(self, address, port=None):
        return self.adb.client_contact(address, port)

    def save_client_contact(self, caller, name=None, new_name=None):
        try:
            result = self.adb.save_client_contact(name, new_name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)

    def add_contact(self, caller, name, address, port=None, auto_created=False):
        try:
            result = self.adb.add(name, address, port, auto_created)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)
        return result

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
            self.notify(caller, (name,kind), 'not_found') # calls self.notify(caller, value, key=None)

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

    def start_connection(self, caller, contact=None):
        if contact == None:
            contact = self.adb.get_current()
        if contact and contact.name in self.adb.contacts:
            try:
                connection = connectors.create_connection(contact, self)
                connection.start()
                result = 'Trying to connect with %s (%s)...' % (contact.name, contact.address)
            except ConnectionError, err:
                result = err
            self.notify(caller, result)
        else:
            self.notify(caller, 'Cannot start connection. No valid contact selected.')

    def stop_connection(self, caller, contact=None):
        if contact == None:
            contact = self.adb.get_current()
        if contact and contact.name in self.adb.contacts:
            result = connectors.stop_connection(contact)
            self.notify(caller, result, 'info')
        else:
            self.notify(caller, 'Cannot stop connection. No valid contact selected.', 'info')

#        self.stop_streams(caller)
#        if self.curr_streams == 'send':
#            contact = self.adb.get_current()
#            connector = self.connectors[contact.kind()]
#            client = connector.disconnect(self, contact.address, contact.port)
#        else:
#            connector = self.connectors[self.connection[2]]
#            client = connector.disconnect(self, self.connection[0], self.connection[1])
#        self.notify(caller, 'Communication was stopped.', 'info')

    def accept_connection(self, caller, connection):
        connection.accept()
        self.notify(caller, 'Begining to receive...', 'info')

    def refuse_connection(self, caller, connection):
        connection.refuse()
        self.notify(caller, 'You refuse the connection.', 'info')

    def set_connection(self, address, port, connector):
        self.connection = (address, port, connector)


    def get_default_port(self, connector):
        return self.connectors[connector].PORT
    
    def devices_list_attributes(self, caller, driver_kind, driver_name, device_name): # TODO: add driver_kind in cli
        """
        :driver_kind: 'video', 'audio' or 'data'
        :driver_name: 'alsa', 'v4l2'
        :device_name: '/dev/video0', 'hw:1'
        """
        # TODO: if IndexError, 
        try: 
            manager = devices.managers[driver_kind]
        except:
            self.notify(caller, 'No such kind of driver: %s' % (driver_kind), 'info')
            return 
        try:
            driver = manager.drivers[driver_name]
        except:
            self.notify(caller, 'No such driver name: %s' % (driver_name), 'info')
            return 
        try:
            device = driver.devices[device_name]
        except:
            self.notify(caller, 'No such device: %s' % (device_name), 'info')
            return 
        self.notify(caller, device.attributes, 'on_devices_list_attributes') # TODO
        #self.notify(caller, 'No such device or driver: %s %s' % (driver_name, device_name), 'info')
        # devices.get_driver(driver_name).devices[device_name].list_attributes()
        #self.notify(caller, 'you called devices_list_attributes', 'devices_list_attributes')

    def devices_modify_attribute(self, caller, driver_kind, driver_name, device_name, attribute_name, value):
        pass
        # TODO
        self.notify(caller, 'No such attributes for driver/device: %s %s %s:%s' % (driver_name, device_name, attribute_name, str(value)), 'info')
        self.notify(caller, 'you called devices_modify_attributes', 'devices_modify_attribute')
        
        # devices.get_driver(driver_name).devices[device_name].attributes[attribute_name].set_value(value)

    def devices_list(self, caller, driver_kind=None, driver_name=None):
        pass
        if driver_name is None:
            self.notify(caller, 'No drivers to list.', 'info')
        else:
            self.notify(caller, 'No such driver: %s' % (driver_name), 'info')
        # devices.get_driver(driver_name).list_devices()
        self.notify(caller, 'you called devices_list', 'devices_list')

