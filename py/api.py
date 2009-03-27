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

# App imports
from errors import *
import connectors
from connectors.states import *
import devices
from devices import firewire
import network
import addressbook # for network_test_*
from protocols import pinger
import repr
import settings

from utils import log
log = log.start('error', 1, 0, 'api') # added by hugo

from twisted.internet import reactor

def modify(who, name_of_who, what, new_value):
    """
    Given an object reference, returns a command that modifies the value of a member
    of that object. this command string can then be used by exec to do its work
    """
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
        #self.network_tester = 
        network.start(self)  # TODO: move to core.
        pinger.start(self)   # TODO: move to core.
        firewire.start(self)
        settings.init_connection_listeners(self)
    ### Contacts ###
    
    def listen_tcp(self, port, factory, listen_queue_size=50, interfaces=''):
        """
        Wraps reactor.listenTCP
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

    def modify_contact(self, caller, name=None, new_name=None, address=None, port=None, setting = None):
        """
        Changes one or more attributes of a contact.
        If no name is given, modify the selected contact.
        """
        try:
            result = self.adb.modify(name, new_name, address, port, setting)
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
            if attribute == 'settings':
                toks = value.partition(':')
                key = toks[0]
                val = toks[2]
                if not val:
                    x.settings.pop(key)
                else:
                    x.settings[key] = val
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

    def start_streams(self, caller, contact_name):
        """
        Starts all the sub-streams. (audio, video and data)
                
        address: string or None (IP)
        """
        log.info("start_streams: ")
        try:
            contact = self.adb.get_contact(contact_name)    
            if contact.state != addressbook.CONNECTED: 
                self.notify(caller, "You must be joined with the contact prior to start streaming.", "error")
            else:
                com_chan = None 
                try:
                    com_chan = contact.connection.com_chan
                except Exception, e:
                    msg = "No com_chan in start_streams(): " + e.message
                    debug.error(msg)
                    self.notify(caller, msg, "error")
                else:
                    remote_addr = contact.address
                    try:
                        settings_com_channel = settings.get_settings_channel_for_contact(contact.name)
                    except KeyError, e:
                        self.notify(caller, "No settings channel for contact", "error")
                    else:
                        id = contact.setting
                        global_setting = self.settings.get_global_setting_from_id(id)
                        address = contact.address
                        global_setting.start_streaming(self, address, settings_com_channel)
                        self.notify(caller, "streaming started") 
        except AddressBookError, e:
                self.notify(caller, "Please select a contact prior to start streaming." + e.message, "error")   
        except SettingsError, err:
            self.notify(caller, err)    
        except StreamsError, err:
            self.notify(caller, err)    
        

    def stop_streams(self, caller, contact_name = None):
        """
        Stop all the sub-streams. (audio, video and data)
        """
        result = True
        try:
            contact = self.adb.contacts[self.adb.selected]
            id = contact.setting
            
            global_setting = self.settings.global_settings[id] # might cause KeyError
            
            address = contact.address
            log.info("stop_streams: ")
            global_setting.stop_streaming() #self, address)
            
        except SettingsError, err:
            result = err
        except KeyError, e:
            log.error("KeyError in stop_streams : " + e.message)
        self.notify(caller, result)   


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
                connection.start()
                # key is 'start_connection'
                # now, we use a dict, not a string in order to give
                # good visual feedback for the web UI.
                # 'context' key could be removed, since we know the 
                # 'key' arg is 'start_connection'
                self.notify(caller, {'name':contact.name, 
                                 'address':contact.address,
                                 'msg':'Trying to connect',
                                 'context':'connection'}) 
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
            result = connectors.stop_connection(contact)
            self.notify(caller, result, 'info')
        else:
            self.notify(caller, 'Cannot stop connection. No valid contact selected.', 'info')

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
    def network_test_start(self, caller, bandwidth=1, duration=10, kind="localtoremote", contact=None):
        """
        Tries to start a network test with currently connected contact.
        
        If dualtest, asks the contact if she wants to test the network
        Starts to test the network using `iperf -c <host> -u [...]`
        
        :param bandwidth: in Mbit
        :param duration: in seconds
        :param kind: string "localtoremote","remotetolocal",  "tradeoff" or "dualtest"
        :param contact: addressbook.Contact object.
        """
        # TODO: accept a contact name instead of contact object ? 
        try:
            if contact is None:
                contact = self.get_contact()
        except AddressBookError:
        #    contact = None
        #if contact is None:
            self.notify(caller, "Please select a contact prior to start a network test.", "error")
        else:
            #pprint.pprint(contact.__dict__)
            
            if contact.state != addressbook.CONNECTED: 
                self.notify(caller, "Please connect to a contact prior to start a network test.", "error")
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
                    self.notify(caller, "No network tester for contact", "error")
                else:
                    try:
                        kind = kinds[kind]
                    except KeyError:
                        self.notify(caller, "Could not start network test: Invalid kind of test \"%s\"." % kind, "error")
                    else:
                        ret = tester.start_test(caller, bandwidth, duration, kind) # TODO: dont need com_chan arg anymore
                        if ret:
                            self.notify(caller, "Starting network performance test with contact %s for %d seconds..." % (contact.name, duration), "info")
                        else:
                            self.notify(caller, "An error occuring while trying to start network test.", "error")

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
        try:
            if contact is None:
                contact = self.get_contact()
        except AddressBookError, e:
            self.notify(caller, "Please select a contact prior to start a pinger test." + e.message, "error")
        else:
            if contact.state != addressbook.CONNECTED: 
                self.notify(caller, "Please connect to a contact prior to start a pinger test.", "error")
            else:
                com_chan = None 
                try:
                    com_chan = contact.connection.com_chan
                except Exception, e:
                    msg = "No com_chan in pinger_start(): " + e.message
                    debug.error(msg)
                    self.notify(caller, msg, "error")
                else:
                    remote_addr = contact.address
                    try:
                        ping = pinger.get_pinger_for_contact(contact.name)
                    except KeyError, e:
                        self.notify(caller, "No pinger for contact", "error")
                    else:
                        ret = ping.start_ping(caller)
                        if ret:
                            self.notify(caller, "Starting pinger test with contact %s" % (contact.name), "info")
                        else:
                            self.notify(caller, "An error occuring while trying to pinger test.", "error")

