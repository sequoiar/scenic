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

from twisted.internet import reactor
from twisted.python import failure

# App imports
from miville.errors import *
from miville import connectors
from miville.connectors.states import *
from miville import devices
from miville.devices import firewire
from miville import network
from miville import addressbook # for network_test_*
from miville.protocols import pinger
from miville.utils import log
from miville.streams import manager as streams_manager
from miville.streams import conf

log = log.start('debug', 1, 0, 'api') # added by hugo

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
        self.core = core
        self.addressbook = core.addressbook
        self.connectors = core.connectors
        self.connection = None
        
        network.start(self, self.core.config.iperf_port + self.core.config.port_numbers_offset, self.core.config.listen_to_interfaces)
        pinger.start(self)
        firewire.start(self)
        conf.PORT_OFFSET = self.core.config.port_numbers_offset * 100 # FIXME
        streams_manager.start(self) # XXX order matters
        self.streams_manager = streams_manager.get_single_manager()
        self.config_db = conf.get_single_db()
        self.devices_toggle_kill_jackd_enabled(self.get_config("restart_jackd")) # XXX : usually you might not want this. In the config it might be False.
    
    def get_config(self, key):
        """
        get the value of a miville configuration option.
        
        might raise KeyError
        """
        return self.core.config.__dict__[key]
 
    ### TCP server ### ------------------------------------------------------
    def listen_tcp(self, port, factory, interfaces='', listen_queue_size=50):
        """
        Wraps reactor.listenTCP

        The order of arguments is different !
        """
        if not isinstance(interfaces, list):
            # '' mean all interfaces
            interface = interfaces
            log.debug("listenTCP:%s %s %s %s" % (port, factory, listen_queue_size, interface))
            log.info("listen TCP on port %s" % (port))
            reactor.listenTCP(port, factory, listen_queue_size, interface)
        else:
            for interface in interfaces:
                log.debug("listenTCP:%s %s %s %s" % (port, factory, listen_queue_size, interface))
                reactor.listenTCP(port, factory, listen_queue_size, interface)

    ### Contacts ### ---------------------------------------------------
    def get_contacts(self, caller):
        """
        Get the list of all the contacts.

        :rtype: a tuple of 2 items:

            - a dictionnary of Contact instances
            - the name of the selected Contact
        """
        self.notify(caller, (self.addressbook.contacts, self.addressbook.selected))

    def get_contact(self, name=None, caller=None):
        """
        Get the named contact instance.
        """
        try:
            result = self.addressbook.get_contact(name)
        except AddressBookError, err:
            result = err
        if caller:
            self.notify(caller, result)
        else:
            return result # self.connectors

    def find_contact(self, address, port=None, connector=None):
#        return self.addressbook.find_contact(address, port)
        """
        Find a contact by its address and port.
        """
        for contact in self.addressbook.contacts.values():
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
        return self.addressbook.client_contact(address, port)

    def save_client_contact(self, caller, name=None, new_name=None, auto_answer=False):
        """
        Saved permanently an auto created contact to the Address Book.
        If no name is given, the selected is use. If new_name is given the
        contact will be saved under this new name.
        """
        try:
            result = self.addressbook.save_client_contact(name, new_name, auto_answer)
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
            result = self.addressbook.add(name, address, port, auto_created, auto_answer)
        except AddressBookError, err:
            result = err
        self.notify(caller, result) # implicit key: 'add_contact'
        return result

    def delete_contact(self, caller, contact_name=None):
        """
        Deletes the named contact from the Address Book or the selected
        if no name is given.
        """
        try:
            contact = self.get_contact(contact_name)
            if isinstance(contact, AddressBookError):
                print sys.exc_info()
                raise contact
            if contact.state == CONNECTED:
                raise AddressBookError('Please disconnect from contact prior to delete it.')
            result = self.addressbook.delete(contact_name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result) # implicit key: 'delete_contact'

    def list_profiles(self, caller):
        """
        Lists all available profiles.
        Notifies the observers with a list of profiles.
        """
        log.debug("list_profiles")
        db = self.config_db
        result = db.profiles.values() # id:profile
        self.notify(caller, result)
    
    def get_profile_details(self, caller, profile_id):
        """
        Lists all available profiles.
        Notifies the observers with a list of profiles.
        """
        # TODO: don't we have anything to send from here?
        log.debug("get_profile_details")
        result = profile_id
        self.notify(caller, result)

    def modify_contact(self, caller, name=None, new_name=None, address=None, port=None, auto_answer=None, profile_id=None):
        """
        Changes one or more attributes of a contact.
        If no name is given, modify the selected contact.
        """
        try:
            result = self.addressbook.modify(name, new_name, address, port, auto_answer, profile_id)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)
    
    
    def modify_contact_attr(self, caller, contact_name, attr_name, attr_value):
        """
        Modifies one and only one attribute of a contact.
        If selected_contact is None, uses the selected contact.
        
        Returns None.
        :param contact_name: unicode or None
        :param attr_name: str
        :attr_value: int or str or unicode or bool
        
        Currently implemented attrbutes ::
          * address : str
          * port : int
          * profile_id : int
          * auto_answer : bool
        """
        try:
            self.addressbook.modify_contact_attr(contact_name, attr_name, attr_value)
            #TODO: what if contact name is None?
            result = (contact_name, attr_name, attr_value)
        except AddressBookError, e:
            result = failure.Failure(e)
        self.notify(caller, result)
        
    def duplicate_contact(self, caller, name=None, new_name=None):
        """
        Adds a copy of the named contact in the Address Book and add the string
        ' (copy)' to is name if no new name is given, else give the new name.
        If no name is given, copy the selected contact.
        """
        try:
            result = self.addressbook.duplicate(name, new_name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)

    def select_contact(self, caller, name):
        """
        Selects one contact in the Address Book (by name). It become the 'active'
        contact.
        """
        try:
            result = self.addressbook.select(name)
        except AddressBookError, err:
            result = err
        self.notify(caller, result)

    def list_streams(self, caller):
        """
        Lists all active streams
        
        Notifies with a list of all streams.
        """
        # TODO: add contact or service arg
        all_streams = self.streams_manager.list_all_streams()
        log.debug("list_streams: %s" % (all_streams))
        notif = all_streams
        self.notify(caller, notif)
    
    def list_streams_for_contact(self, contact):
        """
        Returns the list of streams for a contact.
        :param contact: addressbook.Contact instance.
        """
        return self.streams_manager.list_streams_for_contact(contact)
    
    def start_streams(self, caller, contact_name):
        """
        Starts audio/video streaming with a contact. 
        
        IMPORTANT: there are also remote_start_streams and remote_stopped_streams notification 
        that are called directly from the StreamsManager.
        """
        #TODO: use constants for these !
        # stream_state 0 = stopped
        # stream_state 1 = starting
        # stream_state 2 = streaming
        # stream_state 3 = stopping
        log.info('ControllerApi.start_streams, contact= ' + str(contact_name))
        contact = self.get_contact(contact_name)
        if isinstance(contact, AddressBookError):
            #FIXME: for now we crash. The addressbook should not return exceptions !!!!
            raise contact
        
        contact.stream_state = 1 # STARTING # FIXME
        # TODO: check contact.stream_state
        #if contact.state == CONNECTED:
        #    contact.stream_state = 2 # STARTED
        #elif contact.state == DISCONNECTED:
        #deferred = 
        self.streams_manager.start(contact)
        #def _cb_success(result, self, caller, contact_name):
        #    msg = "Successfully started to stream with %s.\n" % (contact_name)
        #    msg += "%s" % (result)
        #    contact = self.get_contact(contact_name)
        #    contact.stream_state = 2 # STARTED
        #    notif = {
        #        "contact_name":contact_name,
        #        "contact":contact,
        #        "message":msg,
        #        "success":True
        #        }
        #    self.notify(caller, notif, "start_streams")
        #    return result
        #def _eb_failure(err, self, caller, contact_name):
        #    contact = self.get_contact(contact_name)
        #    contact.stream_state = 0 # STOPPED
        #    msg = "Could not start to stream with %s.\n%s" % (contact_name, str(err.getErrorMessage()))
        #    notif = {
        #        "contact_name":contact_name,
        #        "contact":contact,
        #        "message":msg,
        #        "success":False
        #        }
        #    self.notify(caller, notif, "start_streams")
        #    return err
        #deferred.addCallback(_cb_success, self, caller, contact_name)
        #deferred.addErrback(_eb_failure, self, caller, contact_name)
        #return deferred
    
    def remote_stopped_streams(self, caller, notif):
        """
        Called from the StreamsManager when remote host has stopped to stream.
        
        :param caller: Always None
        :param notif: dict with keys  
            {
            "contact":contact_infos.contact,
            "contact_name":contact_name,
            "message":msg,
            "success":False,
            }
        """
        log.info("REMOTE_STOPPED_STREAMS")
        self.notify(caller, notif)
        notif["contact"].stream_state = 0 # FIXME remove this

    def remote_started_streams(self, caller, notif):
        """
        Called from the StreamsManager when remote host has started to stream.
        
        :param caller: Always None
        :param notif: dict with keys  
            {
            "contact":contact_infos.contact,
            "contact_name":contact_name,
            "message":msg,
            "success":False,
            }
        """
        log.info("REMOTE_STARTED_STREAMS")
        if notif["success"]:
            notif["contact"].stream_state = 2 # FIXME remove this
        else:
            notif["contact"].stream_state = 0 # FIXME remove this
        self.notify(caller, notif)
                            
    def stop_streams(self, caller, contact_name):
        """
        Stop all the streams (audio, video and data) for a given contact.
        
        IMPORTANT: there are also remote_start_streams and remote_stopped_streams notification 
        that are called directly from the StreamsManager.
        """
        log.info('ControllerApi.stop_streams, contact= ' + str(contact_name))
        contact = self.get_contact(contact_name)
        # TODO: check contact.state (to send it only once, since javascript sends it twice...)
        if contact.stream_state == 3: #stopping 
            log.error("stop_streams called while already is stopping stream state.")
            return # FIXME: use session instead of contact.stream_state.
        # if contact.stream_state != 2: # streaming
        #     state = "stopped" # 0
        #     if contact.stream_state == 1:
        #         state = "starting"
        #     elif contact.stream_state == 2:
        #         state = "streaming"
        #     elif contact.stream_state == 3:
        #         state = "stopping"
        #     msg = "We are not streaming with contact %s. State is %s." % (contact_name, state)
        #     notif = {
        #         "contact_name":contact_name,
        #         "contact":contact,
        #         "message":msg,
        #         "success":False
        #         }
        #     self.notify(caller, notif, "stop_streams")
        #     return defer.fail(failure.Failure(Exception(msg))) # FIXME
        #deferred = 
        self.streams_manager.stop(contact)
        #contact.stream_state = 3 # STOPPING
        #def _cb_success(result, self, caller, contact_name):
        #    contact = self.get_contact(contact_name)
        #    contact.stream_state = 0 # STOPPED
        #    msg = "Successfully stopped to stream with %s.\n%s" % (contact_name, str(result))
        #    notif = {
        #        "contact_name":contact_name,
        #        "contact":contact,
        #        "message":msg,
        #        "success":True
        #        }
        #    self.notify(caller, notif, "stop_streams")
        #    return result
        #def _eb_failure(err, self, caller, contact_name):
        #    contact = self.get_contact(contact_name)
        #    contact.stream_state = 0 # STOPPED
        #    msg = "Error stopping streaming with %s.\n%s\nThere should be no stream left." % (contact_name, str(err.getErrorMessage()))
        #    notif = {
        #        "contact_name":contact_name,
        #        "contact":contact,
        #        "message":msg,
        #        "success":False
        #        }
        #    self.notify(caller, notif, "stop_streams")
        #    return err
        #deferred.addCallback(_cb_success, self, caller, contact_name)
        #deferred.addErrback(_eb_failure, self, caller, contact_name)
        #return deferred

    ### Connect with com_chan ### ------------------------------------------------------
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
            contact = self.addressbook.get_current() #FIXME : DEPRECATED The CLI should pass a contact argument.
        if contact and contact.name in self.addressbook.contacts:
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
                # TODO return failure
        else:
            self.notify(caller, {'msg':'No contact selected',
                                 'context':'connection'})
            # TODO return failure

    def stop_connection(self, caller, contact=None):
        if contact == None:
            contact = self.addressbook.get_current()
        if contact and contact.name in self.addressbook.contacts:
            try:
                # try stop streams before disconnecting
                if contact.stream_state != 0:
                    self.stop_streams(caller, contact.name)
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
        self.notify(caller, {'connection':connection}, 'accept_connection')

    def refuse_connection(self, caller, connection):
        connection.refuse()
        self.notify(caller, 'You refused the connection.', 'info')

    def set_connection(self, address, port, connector):
        self.connection = (address, port, connector)

    def get_default_port(self, connector):
        """
        Returns the default port for a connector
        """
        return self.connectors[connector].PORT
    
    def get_com_chan_port(self):
        """
        Returns the port for the com_chan
        """
        return self.core.com_chan_port
   
    ### devices ### ----------------------------------------------------------------
    def devices_list_all(self, caller):
        """
        Gets the list of all devices.
        """
        devices_list = []
        for manager in devices.managers.values():
            for driver in manager.drivers.values():
                for device in driver.devices.values():
                    devices_list.append(device)
        self.notify(caller, devices_list, 'devices_list_all')
        # TODO Return a Deferred !

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
        # TODO Return a Deferred !

    def devices_toggle_kill_jackd_enabled(self, enabled=False):
        """
        Enables or disables the auto kill and resurrect jackd when it seems to be frozen.
        Called at startup with the value in the miville options.
        """
        try:
            devices.jackd.toggle_kill_jackd_enabled(enabled)
        except Exception, e:
            log.error(e.message)
        # TODO Return a Deferred !
    
    def set_video_standard(self, caller, value=None):
        """
        Easily sets the video standard. (norm)
        
        Valid string values are "ntsc", "secam" and "pal".
        If value is None, sets it according to the time zone.
        """
        return devices.set_video_standard(caller, value)
        # TODO Return a Deferred !
    
    def reset_firewire_bus(self, caller):
        """
        Resets the firewire (ieee1394) bus.
        """
        # TODO: add bus number ?
        firewire.firewire_bus_reset(caller)
        # TODO Return a Deferred !

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
        # TODO Return a Deferred !

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
        # TODO Return a Deferred !
    
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
                self.notify(caller, "You are not connected to this contact.", "network_test_error")
            else:
                #log.debug("connector : " + str(contact.connector))
                # com_chan = None 
                # try:
                #     com_chan = contact.connection.com_chan
                # except Exception, e:
                #     debug.error("network_test_start(): " + e.message)
                
                #remote_addr = contact.address
                remote_addr = contact.address
                kinds = {
                    "localtoremote":network.KIND_UNIDIRECTIONAL, 
                    "remotetolocal":network.KIND_REMOTETOLOCAL,
                    "dualtest":network.KIND_DUALTEST, 
                    "tradeoff":network.KIND_TRADEOFF, 
                }
                try:
                    tester = network.get_tester_for_contact(contact.name)
                except KeyError:
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
                            # the API has already been notified with network_test_error key
                            pass
                            # self.notify(caller, "An error occuring while trying to start network test.", "network_test_error")
        # TODO Return a Deferred !

    def network_test_abort(self, caller, contact_name=None):
        """
        Interrupts suddenly the network test.
        """
        try:
            if contact_name is None:
                contact = self.get_contact()
            else:
                contact = self.get_contact(contact_name)
        except AddressBookError:
            self.notify(caller, "Please select a contact prior to stop a network test.", "network_test_error")
        else:
            if contact.state != addressbook.CONNECTED: 
                self.notify(caller, "You are not connected to this contact.", "network_test_error")
            else:
                try:
                    tester = network.get_tester_for_contact(contact.name)
                except KeyError:
                    self.notify(caller, "No network tester for contact", "network_test_error")
                else:
                    tester.abort_test(caller)
        # TODO Return a Deferred !
                    
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
            if isinstance(contact, AddressBookError):
                raise contact # FIXME: DO NOT RETURN EXCEPTION PLEASE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
                #com_chan = None 
                self.notify(caller, '...to which we are connected.', 'info')
                try:
                    #com_chan = contact.connection.com_chan
                    self.notify(caller, 'Using a com_chan.', 'info')
                except Exception, e:
                    msg = "No com_chan in pinger_start(): " + e.message
                    log.debug.error(msg)
                    self.notify(caller, msg, "error")
                else:
                    remote_addr = contact.address
                    self.notify(caller, 'Remote IP is %s' % (remote_addr), 'info')
                    try:
                        ping = pinger.get_pinger_for_contact(contact.name)
                    except KeyError:
                        self.notify(caller, "No pinger for contact", "error")
                    else:
                        ret = ping.start_ping(caller)
                        if ret:
                            self.notify(caller, "Starting pinger test with contact %s" % (contact.name), "info")
                        else:
                            self.notify(caller, "An error occuring while trying to do a pinger test.", "error")
        # TODO Return a Deferred !

