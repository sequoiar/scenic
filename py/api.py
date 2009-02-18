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
import network
import addressbook # for network_test_*

import repr

from utils import log
log = log.start('error', 1, 0, 'api') # added by hugo



def modify(who, name_of_who, what, new_value):
    """
    Given an object reference, returns a command that modifies the value of a member
    of that object. this command string can then be used by exec to do its work
    """
    members = dir(who)
    if not what in members:
        raise SettingsError, "Property \"" + attribute + "\" does not exist"
    value = str(new_value)
    if isinstance(new_value, str):
        value = repr.repr(new_value)
    cmd = "%s.%s = %s" % (name_of_who, what, value )
    return cmd    
   
class ControllerApi(object):
    """
    The API of the application. 

    Most of the methods in this class are the use cases of the application. 
    
    It is the "model" in the MVC pattern.
    The controller API that all controllers (such as cli.CliController) must use.
    """

    def __init__(self, notify):
        """
        This class has a notify method inherited (using prototyping) from the utils.observer.Subject class.
        
        Since its notify method is given by the core, all the core's observers observe this class.
        :param notify: the notify method that it will use. 
        """
        self.notify = notify

    def _start(self, core):
        """
        Starts the API once all parts have been loaded.
        
        Some attributes are defined here, but should be defined in __init__ 
        """
        
        self.settings = core.settings
        
        self.core = core
        self.adb = core.adb
#        self.all_streams = core.curr_setting.streams
#        self.curr_streams = 'send'
#        self.streams = self.all_streams[self.curr_streams]
        self.connectors = core.connectors
        self.connection = None
#        self.network_tester = network.start(self) # dict with 'client' and 'server' keys

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
        return self.adb.get_contact(name) # self.connectors

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
        """Returns the port number for a connector. (identified by its name) """
        return self.connectors[connector].PORT

    def client_contact(self, address, port=None):
        """
        Creates a temporary contact for the caller if he is not already in the
        Address Book. Contact are searched by address and port (optional).
        """
        return self.adb.client_contact(address, port)

    def save_client_contact(self, caller, name=None, new_name=None):
        """
        Saved permanently an auto created contact to the Address Book.
        If no name is given, the selected is use. If new_name is given the
        contact will be saved under this new name.
        """
        try:
            result = self.adb.save_client_contact(name, new_name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)

    def add_contact(self, caller, name, address, port=None, auto_created=False):
        """
        Adds a contact to the Address Book.
        
        Name and address are mandatory, port is optional and if connector is
        None, it will be deduce from the address type. Setting is set to 0
        (base setting) by default.

        Address, port and setting are validated and exception are raise
        on problems.
        """
        try:
            result = self.adb.add(name, address, port, auto_created)
        except AddressBookError, err:
            result = err
        self.notify(caller, result) # implicit key: 'add_contact'
        return result

    def delete_contact(self, caller, name=None):
        """
        Deletes the named contact from the Address Book or the selected
        if no name is given.
        """
        try:
            result = self.adb.delete(name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result) # implicit key: 'delete_contact'

    def modify_contact(self, caller, name=None, new_name=None, address=None, port=None):
        """
        Changes one or more attributes of a contact.
        If no name is given, modify the selected contact.
        """
        try:
            result = self.adb.modify(name, new_name, address, port)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)

    def duplicate_contact(self, caller, name=None, new_name=None):
        """
        Adds a copy of the named contact in the Address Book and add the string
        ' (copy)' to is name if no new name is given, else give the new name.
        If no name is given, copy the selected contact.
        """
        try:
            result = self.adb.duplicate(name, new_name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)

    def select_contact(self, caller, name):
        """
        Selects one contact in the Address Book (by name). It become the 'active'
        contact.
        """
        try:
            result = self.adb.select(name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)

    ### Settings ###
    
    def modify_media_stream(self, caller, global_setting_name, stream_subgroup_name, media_stream_name, attribute, new_value ):
        try:
            result = None
            log.info("modify_media_stream")
            glob = self.settings.get_global_setting(global_setting_name)
            sub = glob.get_stream_subgroup(stream_subgroup_name)
            stream = sub.get_media_stream(media_stream_name)
            cmd = modify(stream,'stream', attribute, new_value)
            exec(cmd) 
            #result = True
        except SettingsError, err:
            result = err
        self.notify(caller, result)        
    
    def erase_media_stream (self, caller, global_setting_name, stream_subgroup_name, media_stream_name):
        try:
            result = None
            log.info("erase_media_stream")
            global_setting = self.settings.get_global_setting(global_setting_name)
            stream_subgroup= global_setting.get_stream_subgroup(stream_subgroup_name) 
            result = stream_subgroup.erase_media_stream(media_stream_name)
        except SettingsError, err:
            result = err
        self.notify(caller, result)
            
    def list_media_stream(self, caller, global_setting_name, stream_subgroup_name):
        try:
            result = None
            log.info("list_media_stream")
            global_setting = self.settings.get_global_setting(global_setting_name)
            stream_subgroup= global_setting.get_stream_subgroup(stream_subgroup_name) 
            result =  stream_subgroup.list_media_stream()
        except SettingsError, err:
            result = err
        self.notify(caller, result)

    def add_media_stream (self, caller, type_name, global_setting_name, stream_subgroup_name):
        try:
            result = None
            log.info("add_media_stream")
            glob = self.settings.get_global_setting(global_setting_name)
            subgroup = glob.get_stream_subgroup(stream_subgroup_name)
            result = subgroup.add_media_stream(type_name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
    def erase_stream_subgroup (self, caller, name, global_setting_name, stream_subgroup_name):
        try:
            result = None
            log.info("erase_media_stream")
            glob = self.settings.get_global_setting(global_setting_name)
            subgroup = glob.get_stream_subgroup(stream_subgroup_name)
            result = subgroup.erase_media_stream(type_name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
    
    def list_stream_subgroup (self, caller, global_setting_name):
        try:
            result = None
            log.info("list_stream_subgroup")
            result = self.settings.get_global_setting(global_setting_name).list_stream_subgroup() 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def add_stream_subgroup (self, caller, name, global_setting_name):
        try:
            result = None
            log.info("add_stream_subgroup")
            result = self.settings.get_global_setting(global_setting_name).add_stream_subgroup(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
            
    def erase_stream_subgroup (self, caller, name, global_setting_name):
        try:
            result = None
            log.info("erase_stream_subgroup")
            result = self.settings.get_global_setting(global_setting_name).erase_stream_subgroup(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
    def modify_stream_subgroup (self, caller, global_setting_name,  name, attribute, new_value):
        try:
            result = None
            log.info("modify_stream_subgroup")
            glob = self.settings.get_global_setting(global_setting_name)
            sub = glob.get_stream_subgroup(name)
            cmd = modify(sub,'sub', attribute, new_value)
            exec(cmd) 
            #result = True
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def duplicate_stream_subgroup (self, caller, name, global_setting_name):
        try:
            result = None
            log.info("duplicate_stream_subgroup")
            result = self.settings.get_global_setting(global_setting_name).duplicate_stream_subgroup(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def select_stream_subgroup (self, caller, name, global_setting_name):
        try:
            result = None
            log.info("select_stream_subgroup")
            result = self.settings.get_global_setting(global_setting_name).select_stream_subgroup(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)

        
    def list_media_setting (self, caller):
        try:
            result = None
            log.info("list_media_setting")
            result = self.settings.list_media_setting() 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def add_media_setting (self, caller, name):
        try:
            result = None
            log.info("add_media_setting")
            result = self.settings.add_media_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)      
        
    def erase_media_setting (self, caller, name):
        try:
            result = None
            log.info("erase_media_setting")
            result = self.settings.erase_media_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def modify_media_setting (self, caller, name, attribute, value):
        try:
            result = None
            log.info("modify_media_setting")
            x = self.settings.get_media_setting(name)
            cmd = modify(x, 'x', attribute, value)
            exec(cmd) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def duplicate_media_setting (self, caller, name):
        try:
            result = None
            log.info("duplicate_media_setting")
            result = self.settings.duplicate_media_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def select_media_setting (self, caller, name):
        try:
            result = None
            log.info("select_media_setting")
            result = self.settings.select_media_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def keep_media_setting (self, caller):
        try:
            result = None
            log.info("keep_media_setting")
            result = self.settings.keep_media_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
        
    def list_global_setting (self, caller):
        try:
            result = None
            log.info("list_global_settings")
            result = self.settings.list_global_setting() 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def add_global_setting (self, caller, name):
        try:
            result = None
            log.info("add_global_setting")
            result = self.settings.add_global_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def type_global_setting (self, caller, name):
        try:
            result = None
            log.info("type_global_setting")
            result = self.settings.type_global_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def erase_global_setting (self, caller, name):
        try:
            result = None
            log.info("erase_global_setting")
            result = self.settings.erase_global_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def modify_global_setting (self, caller, global_setting_name, attribute, value):
        try:
            result = None
            log.info("modify_global_setting")
            glob = self.settings.get_global_setting(global_setting_name)
            # result = glob.modify_global_setting(attribute, value)
            cmd = modify(glob, 'glob', attribute, value)
            exec(cmd) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def duplicate_global_setting (self, caller, name):
        try:
            result = None
            log.info("duplicate_global_setting")
            result = self.settings.duplicate_global_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def select_global_setting (self, caller, name):
        try:
            result = None
            log.info("select_global_setting")
            result = self.settings.select_global_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)
    
    def load_settings (self, caller):
        try:
            result = None
            log.info("load_global_setting")
            result = self.settings.load() 
        except SettingsError, err:
            result = err
        self.notify(caller, result)       
        
    def save_settings (self, caller):
        try:
            result = None
            log.info("save_global_setting")
            result = self.settings.save()
        except SettingsError, err:
            result = err
        self.notify(caller, result)
        
        
    def description_global_setting (self, caller, name):
        try:
            result = None
            log.info("description_global_setting")
            result = self.settings.description_global_setting(name) 
        except SettingsError, err:
            result = err
        self.notify(caller, result)      
        

    ### Streams ###

    def start_streams(self, caller, address, channel=None):
        """
        Starts all the sub-streams. (audio, video and data)
                
        address: string or None (IP)
        """
        self.notify(caller, self.streams.start(address, channel))

    def stop_streams(self, caller):
        """
        Stop all the sub-streams. (audio, video and data)
        """
        self.notify(caller, self.streams.stop())

    def set_streams(self, caller, attr, value):
        """
        Sets an attribute (settings) for the streams manager. 
            
        :param name: string
        :param value: 
        
        Attributes names of the Streas class are : mode, container, port
        """
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
    
    def get_com_chan_port(self):
        return self.core.com_chan_port
   
    ### devices ###

    def device_list_attributes(self, caller, driver_kind, driver_name, device_name): 
        """
        :param driver_kind: 'video', 'audio' or 'data'
        :param driver_name: 'alsa', 'v4l2'
        :param device_name: '/dev/video0', 'hw:1'
        """
        # TODO: updatre CLI to correct method name.
        try:
            attributes = devices.list_attributes(caller, driver_kind, driver_name, device_name)
        except DeviceError, e:
            self.notify(caller, e.message, 'info')
        else:
            self.notify(caller, attributes, 'device_list_attributes') # dict

    def device_modify_attribute(self, caller, driver_kind, driver_name, device_name, attribute_name, value):
        """
        Modifies a device's attribute
        
        
        :param driver_kind: 'video', 'audio' or 'data'
        :param driver_name: 'alsa', 'v4l2'
        :param device_name: '/dev/video0', 'hw:1'
        :param attribute_name:
        """
        try:
            devices.modify_attribute(caller, driver_kind, driver_name, device_name, attribute_name, value)
        except DeviceError, e:
            self.notify(caller, e.message, 'info') # TODO: there should be a 'user_error' key.
        # TODO: modify method name in CLI

    def devices_list(self, caller, driver_kind):
        """
        Gets the list of devices for a driver_kind.

        :param driver_kind: 'video', 'audio' or 'data'
        """
        
        try: 
            manager = devices.managers[driver_kind]
        except KeyError:
            self.notify(caller, 'No such kind of driver: %s' % (driver_kind), 'info')
            return

        #self.notify(caller, 'you called devices_list', 'devices_list')
        # TODO: make asynchronous
        devices_list = []
        for driver in manager.drivers.values():
            for device in driver.devices.values():
                devices_list.append(device)
        self.notify(caller, devices_list, 'devices_list') # with a dict
    
    ### devices ###

    def device_list_attributes(self, caller, driver_kind, driver_name, device_name): 
        """
        :param driver_kind: 'video', 'audio' or 'data'
        :param driver_name: 'alsa', 'v4l2'
        :param device_name: '/dev/video0', 'hw:1'
        """
        # TODO: updatre CLI to correct method name.
        try:
            attributes = devices.list_attributes(caller, driver_kind, driver_name, device_name)
        except DeviceError, e:
            self.notify(caller, e.message, 'info')
        else:
            self.notify(caller, attributes, 'device_list_attributes') # dict

    def device_modify_attribute(self, caller, driver_kind, driver_name, device_name, attribute_name, value):
        """
        Modifies a device's attribute
        
        
        :param driver_kind: 'video', 'audio' or 'data'
        :param driver_name: 'alsa', 'v4l2'
        :param device_name: '/dev/video0', 'hw:1'
        :param attribute_name:
        """
        try:
            devices.modify_attribute(caller, driver_kind, driver_name, device_name, attribute_name, value)
        except DeviceError, e:
            self.notify(caller, e.message, 'info') # TODO: there should be a 'user_error' key.

        # TODO: modify method name in CLI

    def devices_list(self, caller, driver_kind):
        try: 
            manager = devices.managers[driver_kind]
        except KeyError:
            self.notify(caller, 'No such kind of driver: %s' % (driver_kind), 'info')
            return

        #self.notify(caller, 'you called devices_list', 'devices_list')
        # TODO: make asynchronous
        devices_list = []
        for driver in manager.drivers.values():
            for device in driver.devices.values():
                devices_list.append(device)
        self.notify(caller, devices_list, 'devices_list') # with a dict
    
    # network test use cases ----------------------------------------------------
    def network_test_start(self, caller, bandwidth=1, duration=10, kind="unidirectional"):
        """
        Tries to start a network test with currently connected contact.
        
        If dualtest, asks the contact if she wants to test the network
        Starts to test the network using `iperf -c <host> -u [...]`
        
        :param bandwidth: in Mbit
        :param duration: in seconds
        :param kind: string "unidirectional", "tradeoff" or "dualtest"
        """
        try:
            contact = self.get_contact()
        except AddressBookError:
        #    contact = None
        #if contact is None:
            self.notify(caller, "Please select a contact prior to start a network test.", "error")
        else:
            if contact.state != addressbook.CONNECTED and kind == "dualtest":
                self.notify(caller, "Please connect to a contact prior to start a network test.", "error")
            else:
                if kind == "dualtest":
                    com_chan = contact.connector.com_chan    
                else:
                    com_chan = None
                remote_addr = contact.address
                # TODO
                kinds = {
                    "unidirectional":network.KIND_UNIDIRECTIONAL, 
                    "dualtest":network.KIND_DUALTEST_CLIENT, 
                    "unidirectional":network.KIND_UNIDIRECTIONAL, 
                }
                try:
                    kind = kinds[kind]
                except KeyError:
                    self.notify(caller, "Could not start network test: Invalid kind of test \"%s\"." % kind, "error")
                self.network_tester.start_test(caller, remote_addr, bandwidth, duration, kind, com_chan)
                #client.start_client(caller, server_addr, bandwidth, duration)
                self.notify(caller, "Starting network performance test with contact %s" % (contact.name), "info")
    
    def network_test_stop(self, caller):
        """
        Interrupts suddenly the network test.
        """
        #TODO
        pass

    def network_test_enable_autoaccept(self, caller, enabled=True):
        """
        Enables/disables auto accept of network dualtests from remote selected contact.
        :param enabled: wheter to enable it or not.
        """
        # TODO
        pass

    def network_test_accept(self, caller, accepted=True):
        """
        Accepts/refuses network dualtest asked from selected contact.
        :param accepted: wheter to accept it or not.
        """
        # TODO
        pass

