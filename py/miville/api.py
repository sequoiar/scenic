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
Public API of Miville

Errors managment in notify
==========================
In the new design, errors are sent to the observers as a dict. 
It contains the following keys : 
 * 'msg' : a string describing the error in a human-readable way.
 * 'context': a string referring to a section of this software. 
 * 'exception': should contain a string representing the error. (and not the exception itself?)
For now, the contexts in Miville are : 
 * devices, network, settings, connection, streams, system

The idea is to use only built-in types so that it is easy to convert to javascript. (no python objects)

Example ::
self.api.notify(
    caller, 
    {
    'address':self.contact.address, 
    'port':self.contact.port,
    'exception':'%s' % err,
    'msg':'Connection failed',
    'context':'connection'
    }, 
    "error")
"""

import sys
import pprint 
import repr

from twisted.internet import reactor

# App imports
from miville.errors import *
from miville import connectors
from miville.connectors.states import *
from miville import devices
from miville.devices import firewire
from miville import network
from miville import addressbook # for network_test_*
from miville.protocols import pinger
from miville import settings
from miville import engines
from miville.utils import log
from miville.utils.common import string_to_number 

log = log.start('debug', 1, 0, 'api') # added by hugo


    
def modify(who, name_of_who, what, new_value):
    """
    Given an object reference, returns a command that modifies the value of a member
    of that object. this command string can then be used by exec to do its work
    """
    # TODO: remove this function from here please ! 
    members = dir(who)
    if not what in members:
        raise SettingsError, "Property \"" + what + "\" does not exist"
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
        network.start(self, self.core.config.iperf_port + self.core.config.port_numbers_offset, self.core.config.listen_to_interfaces)
        pinger.start(self)
        firewire.start(self)
        engines.init_connection_listeners(self)
    
    def listen_tcp(self, port, factory, interfaces='', listen_queue_size=50):
        """
        Wraps reactor.listenTCP

        The order of arguments is different !
        """
        if not isinstance(interfaces, list):
            # '' mean all interfaces
            interface = interfaces
            log.debug("listenTCP:%s %s %s %s" % (port, factory, listen_queue_size, interface))
            reactor.listenTCP(port, factory, listen_queue_size, interface)
        else:
            for interface in interfaces:
                log.debug("listenTCP:%s %s %s %s" % (port, factory, listen_queue_size, interface))
                reactor.listenTCP(port, factory, listen_queue_size, interface)


    ### Contacts ###
    def get_contacts(self, caller):
        """
        Get the list of all the contacts.

        :rtype: a tuple of 2 items:

            - a dictionnary of Contact instances
            - the name of the selected Contact
        """
        self.notify(caller, (self.adb.contacts, self.adb.selected))

    def get_contact(self, name=None, caller=None):
        """
        Get the named contact instance.
        """
        try:
            result = self.adb.get_contact(name)
        except AddressBookError, err:
            result = err
        if caller:
            self.notify(caller, result)
        else:
            return result # self.connectors

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

    def save_client_contact(self, caller, name=None, new_name=None, auto_answer=False):
        """
        Saved permanently an auto created contact to the Address Book.
        If no name is given, the selected is use. If new_name is given the
        contact will be saved under this new name.
        """
        try:
            result = self.adb.save_client_contact(name, new_name, auto_answer)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)

    def add_contact(self, caller, name, address, port=None, auto_created=False, auto_answer=False):
        """
        Adds a contact to the Address Book.
        
        Name and address are mandatory, port is optional and if connector is
        None, it will be deduce from the address type. Setting is set to 0
        (base setting) by default.

        Address, port and setting are validated and exception are raise
        on problems.
        """
        try:
            result = self.adb.add(name, address, port, auto_created, auto_answer)
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

    def modify_contact(self, caller, name=None, new_name=None, address=None, port=None, auto_answer=None, setting=None):
        """
        Changes one or more attributes of a contact.
        If no name is given, modify the selected contact.
        """
        try:
            result = self.adb.modify(name, new_name, address, port, auto_answer, setting)
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
    
    def get_config(self, key):
        """
        get the value of a miville configuration option.
        """
        return self.core.config.__dict__[key]

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
            if attribute == 'settings':
                key, sep, string_value = value.partition(':')
#                key = toks[0]
#                string_value = toks[2]
                # this removes the setting element because the 
                # content is None
                if not string_value:
                    x.settings.pop(key)
                else:
                    # first, check for a number:
                    num = string_to_number(string_value)
                    if num == None:
                        x.settings[key] = string_value
                    else:
                        x.settings[key] = num
            else:
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
        
    def pretty_list_settings(self, caller):    
        try:
            result = None
            log.info("pretty_list_settings")
            result = self.settings.pretty_list_settings() 
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
            try:
                exec(cmd)
            except Exception, err:
              result = err
              self.notify(caller, result)   
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
        except InstallFileError, err:
            result = err
        self.notify(caller, result)       
        
    def save_settings (self, caller):
        log.info("ControllerApi.save_settings")
        try:
            result = None
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
    
    def start_streams_tmp(self, caller, contact_name):
        contact = self.get_contact(contact_name)
        if contact:
            if contact.state == CONNECTED:
                contact.stream_state = 2 # STARTED
            elif contact.state == DISCONNECTED:
                deferred = self.start_connection(caller, contact)
                if deferred:
                    deferred.addCallback(self.start_streams_from_defer, caller, contact_name)
                    deferred.addErrback(self.start_connection_error_from_defer, caller, contact_name)
    
    def start_streams_from_defer(self, defer_result, caller, contact_name):
#        self.start_streams_tmp(caller, contact_name)
        self.start_streams(caller, contact_name)
    
    def start_connection_error_from_defer(self, error, caller, contact_name):
        log.debug('Defer error: %s' % error)
        self.notify(caller, error)
    
    def stop_streams_tmp(self, caller, contact_name):
        contact = self.get_contact(contact_name)
        contact.stream_state = 0        
        
    def start_streams(self, caller, contact_name):
        log.info('ControllerApi.start_streams, contact= ' + str(contact_name))
        contact = self.get_contact(contact_name)
        if contact:
            if contact.state == CONNECTED:
                try:
                    contact, global_setting, settings_com_channel  = self._get_gst_com_chan_from_contact_name(contact_name)
                    settings_com_channel.start_streaming( global_setting, contact.address)
                    # global_setting.start_streaming(self, contact.address, settings_com_channel)
                    contact.stream_state = 2
                    self.notify(caller, "streaming started")
                except AddressBookError, e:
                        self.notify(caller, "Please select a contact prior to start streaming." + e.message, "error")   
                except SettingsError, err:
                    self.notify(caller, err)
                except StreamsError, err:
                    self.notify(caller, err)
            elif contact.state == DISCONNECTED:
                deferred = self.start_connection(caller, contact)
                if deferred:
                    deferred.addCallback(self.start_streams_from_defer, caller, contact_name)
                    deferred.addErrback(self.start_connection_error_from_defer, caller, contact_name)
                            
    def stop_streams(self, caller, contact_name):
        """
        Stop all the sub-streams. (audio, video and data)
        """
        log.info('ControllerApi.start_streams, contact= ' + str(contact_name))
        try:
            contact, global_setting, settings_com_channel  = self._get_gst_com_chan_from_contact_name(contact_name)
            settings_com_channel.stop_streaming( contact.address)
            contact.stream_state = 0
            self.notify(caller, "streaming stopped")
        except AddressBookError, e:
                self.notify(caller, "Please select a contact prior to stop streaming." + e.message, "error")   
        except SettingsError, err:
            self.notify(caller, err)
        except StreamsError, err:
            self.notify(caller, err)
                        
    def _get_gst_com_chan_from_contact_name(self, contact_name):
        """
        Starts all the sub-streams. (audio, video and data)
                
        address: string or None (IP)
        """
        
        caller = None
        contact = self.adb.get_contact(contact_name)    
        if contact.state != addressbook.CONNECTED:
            raise AddressBookError, "You must be joined with the contact prior to start streaming."
        try:
            settings_com_channel = engines.get_channel_for_contact('Gst',contact_name)
        except KeyError, e:
            raise StreamsError, "No settings channel for contact"
        id  = contact.setting
        global_setting = self.settings.get_global_setting_from_id(id)
        return contact, global_setting, settings_com_channel 

    ### Connect ###
    def start_connection(self, caller, contact=None):
        """
        Connects (JOIN) to a contact. 

        This step is mandatory prior to :
         * communicate using a com_chan
         * do network tests
         * start a stream
        
        The following steps are then done : 
         * create a connector
         * create a connection if connector was succesful
         * create a com_chan
         * register com_chan callbacks 
        """
        if contact == None:
            contact = self.adb.get_current()
        if contact and contact.name in self.adb.contacts:
            try:
                connection = connectors.create_connection(contact, self)
                deferred = connection.start()
                # key is 'start_connection'
                # now, we use a dict, not a string in order to give
                # good visual feedback for the web UI.
                # 'context' key could be removed, since we know the 
                # 'key' arg is 'start_connection'
                self.notify(caller, {'name':contact.name, 
                                     'address':contact.address,
                                     'msg':'Trying to connect',
                                     'context':'connection'})
                return deferred
            except ConnectionError, err:
                self.notify(caller, {'name':contact.name, 
                                     'address':contact.address,
                                     'exception':'%s' % err,
                                     'msg':'Connection failed',
                                     'context':'connection'})
        else:
            self.notify(caller, {'msg':'No contact selected',
                                 'context':'connection'})

    def stop_connection(self, caller, contact=None):
        if contact == None:
            contact = self.adb.get_current()
        if contact and contact.name in self.adb.contacts:
            try:
                connectors.stop_connection(contact)
                self.notify(caller, {'msg':'Connection stopped',
                                     'name':contact.name})
            except ConnectionError, err:
                self.notify(caller, {'msg':'Cannot stop connection',
                                     'name':contact.name,
                                     'exception':'%s' % err})
        else:
            self.notify(caller, {'msg':'Cannot stop connection',
                                 'exception':'No contact selected'})

    def accept_connection(self, caller, connection):
        connection.accept()
        self.notify(caller, 'Now connected.', 'info')

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

    def devices_toggle_kill_jackd_enabled(self, enabled=False):
        """
        Enables or disables the auto kill and resurrect jackd when it seems to be frozen.
        """
        try:
            devices.jackd.toggle_kill_jackd_enabled(enabled)
        except Exception, e:
            log.error(e.message)
    
    def set_video_standard(self, caller, value=None):
        """
        Easily sets the video standard. (norm)
        
        Valid string values are "ntsc", "secam" and "pal".
        If value is None, sets it according to the time zone.
        """
        return devices.set_video_standard(caller, value)
    
    def reset_firewire_bus(self, caller):
        """
        Resets the firewire (ieee1394) bus.
        """
        # TODO: add bus number ?
        firewire.firewire_bus_reset(caller)

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
    def network_test_start(self, caller, bandwidth=1, duration=10, kind="localtoremote", contact=None, unit='M'):
        """
        Tries to start a network test with currently connected contact.
        
        If dualtest, asks the contact if she wants to test the network
        Starts to test the network using `iperf -c <host> -u [...]`
        
        :param bandwidth: in Mbit
        :param duration: in seconds
        :param kind: string "localtoremote","remotetolocal",  "tradeoff" or "dualtest"
        :param contact: addressbook.Contact object.
        :param unit: either 'M' or 'K' for megabits and kilobits.
        """
        unit = unit.strip().upper()
        if unit == 'M':
            unit = network.MEGABITS
        elif unit == 'K':
            unit = network.KILOBITS
        else:
            unit = network.MEGABITS
            log.error('network_test_start: invalid unit : ' + unit + ' Changed to M.')
        # TODO: accept a contact name instead of contact object ? 
        try:
            if contact is None:
                contact = self.get_contact()
        except AddressBookError:
        #    contact = None
        #if contact is None:
            self.notify(caller, "Please select a contact prior to start a network test.", "network_test_error")
        else:
            #pprint.pprint(contact.__dict__)
            
            if contact.state != addressbook.CONNECTED: 
                self.notify(caller, "Please connect to a contact prior to start a network test.", "network_test_error")
            else:
                #log.debug("connector : " + str(contact.connector))
                # com_chan = None 
                # try:
                #     com_chan = contact.connection.com_chan
                # except Exception, e:
                #     debug.error("network_test_start(): " + e.message)
                
                remote_addr = contact.address
                kinds = {
                    "localtoremote":network.KIND_UNIDIRECTIONAL, 
                    "remotetolocal":network.KIND_REMOTETOLOCAL,
                    "dualtest":network.KIND_DUALTEST, 
                    "tradeoff":network.KIND_TRADEOFF, 
                }
                try:
                    tester = network.get_tester_for_contact(contact.name)
                except KeyError, e:
                    self.notify(caller, "No network tester for contact", "network_test_error")
                else:
                    try:
                        kind = kinds[kind]
                    except KeyError:
                        self.notify(caller, "Could not start network test: Invalid kind of test \"%s\"." % kind, "network_test_error")
                    else:
                        log.debug('iperf bandwidth unit : ' + unit)
                        log.debug('network_test_start %s %s %s %s %s' %  (caller, bandwidth, duration, kind, unit))
                        ret = tester.start_test(caller, bandwidth, duration, kind, unit)
                        # TODO: dont need com_chan arg anymore
                        if ret:
                            log.debug("Will notify observer that we are starting a network test")
                            self.notify(caller, "Starting network performance test with contact %s for %d seconds..." % (contact.name, duration), "network_test_start")
                        else:
                            self.notify(caller, "An error occuring while trying to start network test.", "network_test_error")

#     def network_test_stop(self, caller):
#         """
#         Interrupts suddenly the network test.
#         """
#         #TODO
#         pass
# 
#     def network_test_enable_autoaccept(self, caller, enabled=True):
#         """
#         Enables/disables auto accept of network dualtests from remote selected contact.
#         :param enabled: wheter to enable it or not.
#         """
#         # TODO
#         pass
# 
#     def network_test_accept(self, caller, accepted=True):
#         """
#         Accepts/refuses network dualtest asked from selected contact.
#         :param accepted: wheter to accept it or not.
#         """
#         # TODO
#         pass

    def pinger_start(self, caller, contact=None):
        """
        Tries to start a pinger test.
        
        :param contact: addressbook.Contact object.
        """
        self.notify(caller, 'Trying to ping... ', 'info')
        try:
            if contact is None:
                contact = self.get_contact()
                self.notify(caller, '...with contact %s.' % (contact.name), 'info')
        except AddressBookError, e:
            self.notify(caller, "Please select a contact prior to start a pinger test." + e.message, "error")
        else:
            connected = False
            try:
                if contact.state != addressbook.CONNECTED: 
                    self.notify(caller, "Please connect to a contact prior to start a pinger test.", "error")
                else:
                    connected = True
            except NameError:
                self.notify(caller, "Please connect to a contact prior to start a pinger test.", "error")
            if connected:
                com_chan = None 
                self.notify(caller, '...to which we are connected.', 'info')
                try:
                    com_chan = contact.connection.com_chan
                    self.notify(caller, 'Using a com_chan.', 'info')
                except Exception, e:
                    msg = "No com_chan in pinger_start(): " + e.message
                    debug.error(msg)
                    self.notify(caller, msg, "error")
                else:
                    remote_addr = contact.address
                    self.notify(caller, 'Remote IP is %s' % (remote_addr), 'info')
                    try:
                        ping = pinger.get_pinger_for_contact(contact.name)
                    except KeyError, e:
                        self.notify(caller, "No pinger for contact", "error")
                    else:
                        ret = ping.start_ping(caller)
                        if ret:
                            self.notify(caller, "Starting pinger test with contact %s" % (contact.name), "info")
                        else:
                            self.notify(caller, "An error occuring while trying to do a pinger test.", "error")

