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
Communication channel for the streaming infos between two miville contacts.
"""
#FIXME: using contact name as key?! We should use an other key

import sys
from twisted.internet import reactor
import pprint
import warnings 

#XXX: does import miville.streams.manager within a method/function below !
from miville.utils import log
from twisted.internet import defer
from miville.utils.commands import find_command
from miville.errors import * # StreamError

#from miville import streams
#from miville.services import milhouseservice
#from miville.services.milhouseservice import GstError

log = log.start('debug', 1, 0, 'streams.communication')

# module variables
_services_channels_dict = {}
_api = None

# available streaming services
MILHOUSE_SERVICE_ID = "MILHOUSE"
SERVICES_COM_CHANNEL_NAME = "SERVICES"
# Notitification for stream process state change (1 notif per streaming session)
#  timeout: tbd 5 sec 
#  info: state, contact name, error message
#        states are: stopped, starting, started

def set_api(api):
    """
    Public module function
    """
    global _api
    _api = api
  
def on_com_chan_connected(connection_handle, role="client"):
    """
    Called when a new connection with a contact is made.
    Called from the Connector class in its com_chan_started_client or com_chan_started_server method.
    registers the com_chan callback for settings transferts.
    
    :param connection_handle: connectors.Connection object.
    :param role: string "client" or "server"
    We actually do not care if this miville is a com_chan client or server.
    """
    # TODO: do not use contact.name for key, but id(contact)
    from miville.streams import manager
    global _api
    global _services_channels_dict
    log.debug("on_com_chan_connected")
    contact = connection_handle.contact
    subchan = StreamsCommunication(
        contact=contact, 
        com_chan = connection_handle.com_chan,
        api = _api,
        remote_addr = contact.address, 
        streams_manager = manager.get_single_manager()
        )
    callback = subchan.on_remote_message
    key = _make_key_for_contact(subchan.contact) 
    _services_channels_dict[key] = subchan 
    subchan.com_chan.add(callback, SERVICES_COM_CHANNEL_NAME) # IMPORTANT ! Add this subchan to the com_chan
    log.debug("on_com_chan_connected: streams_subchannels: " + str(_services_channels_dict))
    # exchange IP addresses as seen by the remote contact.
    #TODO: it seems like the SERVICES_COM_CHANNEL_NAME com_chan key is not yet registered at this time.
    subchan.send_message('SEND_ME_MY_IP', [contact.address]) # and tell the other one what's his IP.

def on_com_chan_disconnected(connection_handle):
    """
    Called when a connection is stopped
    """
    global _services_channels_dict
    try:
        del _services_channels_dict[connection_handle.contact.name]
        #log.debug("on_com_chan_disconnected: settings_chans: " + str(_services_channels_dict))
    except KeyError, e:
        log.error("error in on_com_chan_disconnected : KeyError " + e.message)        

#TODO: remove this ?
def get_all_services_channels():
    global _services_channels_dict
    return _services_channels_dict
    
def _make_key_for_contact(contact):
    log.warning("Using contact name as key for communication subchannels.")
    #TODO: use addr:port as key?
    return contact.name

def get_channel_for_contact(contact):
    """
    Returns a StreamsCommunicaiton instance for the starting of streams.
    :param contact: miville.addressbook.Contact object.
    """
    global _services_channels_dict 
    key = _make_key_for_contact(contact)
    if not _services_channels_dict.has_key(key):
        log.error("Comm subchan doesnt have key %s" % (key))
        log.debug("on_com_chan_connected: streams_subchannels: " + str(_services_channels_dict))
        #TODO: this make the CLI crash when called from Bravo !
        #TODO: there is a race condition when creating the communication channel
        raise StreamError("You are not connected to contact %s." % (key))
    # log.debug("get_services_channel_for_contact %s" % (key))
    return _services_channels_dict[key]

class StreamsCommunication(object):
    """
    Allows to send and receive setting information
    via the com_chan protocol, once we are joined to
    a remote contact
    """
    CMD_START = "start"
    CMD_STOP = "stop"
    CMD_SEND_IP = "SEND_ME_MY_IP"
    CMD_YOUR_IP = "YOUR_IP_IS"
    
    def __init__(self, 
            com_chan=None, 
            contact=None, 
            streams_manager=None, 
            api=None, 
            caller=None, 
            remote_addr=None, 
            my_addr=None
            ):
        log.debug('StreamsCommunication.__init__: ' + str(self))
        self.com_chan = com_chan
        self.contact = contact
        self.api = api
        self.caller = caller
        self.remote_addr = remote_addr
        self.my_addr = my_addr
        self.streams_manager = streams_manager
            
    def on_remote_message(self, key, args=None):
        """
        Called by the com_chan whenever a message is received from the remote contact 
        through the com_chan with the "pinger" key.
        
        :param key: string defining the action/message
        :param *args: list of arguments
        
        The key and *args can be one of the following...
        """
        caller = None
        #log.debug('StreamsCommunication.on_remote_message: %s : %s' %  (key, str(args) )  )
        log.debug('on_remote_message: %s' %  (key))
        if key == self.CMD_SEND_IP:
            try:
                self.my_addr = args[0]
            except KeyError, e:
                log.error("IP address is not given in the message !")
                raise # !!
            log.debug("Received %s %s" % (key, args[0]))
            self.send_message(self.CMD_YOUR_IP, [self.remote_addr])
        elif key == self.CMD_YOUR_IP:
            try:
                self.my_addr = args[0]
                log.debug('my ip is ' + str(self.my_addr))
            except KeyError, e:
                log.error("IP address is not given in the message !")
                raise # !!
            log.debug("Received %s %s" % (key, args[0]))
        elif key == self.CMD_START:
            log.debug("received START: starting...")
            try:
                alice_entries = args[0]
                bob_entries = args[1]
            except IndexError, e:
                # TODO: will this crash the whole miville?
                msg = "Error parsing comchan message %s. No configuration entries." % (self.CMD_START)
                log.error(msg)
                raise StreamError(msg)
            else:
                deferred_list = self.streams_manager._cb_start(self, self.contact, alice_entries, bob_entries)
                #TODO: use this deferred list...
        elif key == self.CMD_STOP:
            #TODO
            log.debug("received STOP: stopping...")
            deferred_list = self.streams_manager._cb_stop(self, self.contact)
            #TODO: use this deferred list...
        else:
            log.error("Unknown message in settings channel: " + key +  " args: %s"  % str(args) )
    
    def send_start(self, contact_infos, alice_entries, bob_entries):
        """
        Called by the alice of a streaming session.
        
        called from the miville.streams.manager.StreamsManager.start
        :param alice_entries: dict of configuration entries
        :param bob_entries: dict of configuration entries
        """
        log.debug("send_start")
        #log.debug("send_start %s %s %s" % (contact_infos, alice_entries, bob_entries))
        self.send_message(self.CMD_START, [alice_entries, bob_entries])
        #return defer.succeed(True) # TODO

    def send_stop(self, contact_infos):
        """
        called from the miville.streams.manager.StreamsManager.stop
        """
        log.debug("send_stop")
        # log.debug("send_stop %s" % (contact_infos))
        self.send_message(self.CMD_STOP, [])
        #return defer.succeed(True) # TODO

    def send_message(self, key, args_list=[]):
        """
        Sends a message to the current remote contact through the com_chan with key "network_test".
        :param args: list
        """
        #log.debug("StreamsCommunication._send_message %s. %r" % (key, args_list))
        log.debug("StreamsCommunication._send_message %s. ..." % (key))
        try:
            # list with key as 0th element
            self.com_chan.callRemote(SERVICES_COM_CHANNEL_NAME, key, args_list)
        except AttributeError, e:
            log.error("send_message: Could not send message to remote: " + e.message)
            raise

