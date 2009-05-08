#!/usr/bin/env python 
# -*- coding: utf-8 -*-

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
Ping tests using the com_chan.

Example for Miville's com_chan.
Intended for unit tests.
"""

import os
import sys
import time
import warnings 

from twisted.internet import reactor
from twisted.internet import protocol
from twisted.internet import defer
from twisted.internet.error import ProcessExitedAlready, AlreadyCancelled
from twisted.python import failure
from twisted.protocols import basic

# App imports
from miville.utils import log
from miville.utils import commands
from miville import connectors

log = log.start('debug', 1, False, 'pinger') # LOG TO FILE = False

_pingers = {}
_api = None

class PingerError(Exception):
    """
    Any error due to the pinger
    """
    pass

class PingerTester(object):
    """
    Simple ping usinbg the com_chan
    """
    def __init__(self):
        self.com_chan = None
        self.contact = None
        self.api = None
        self.caller = None
        self.remote_addr = None

    def notify_api(self, caller, key, val):
        """
        little wrapper for api.notify in order to easily debug this module.
        
        calls api.notify
        
        ways that this class has got called : 
        - api.network_test_start(self, caller, bandwidth=1, duration=1)
        """
        if self.api is None:
            if key == "stats":
                print "STATS: ", val
                print "(caller is %s)" % (caller)
            elif key == "error":
                print "ERROR !", val
        else:
            self.api.notify(caller, val, key) #key, res)

    def _get_remote_addr(self):
        """
        Returns the IP of the current remote contact.

        Useful for dualtest case.
        """
        return self.contact.address
        # TODO : make more sturdy.
        ret = None
        contact = self.api.get_contact()
        try:
            ret = contact.address
        except:
            pass
        return ret

    def start_ping(self, caller):
        """
        Method called from the core API to try to start a ping test.
        
        :return success: boolean
        """
        self.caller = caller
        self._send_message("ping")
        self.notify_api(self.caller, 'info', "Sending ping")
        return True

    def on_remote_message(self, key, args):
        """
        Called by the com_chan whenever a message is received from the remote contact 
        through the com_chan with the "pinger" key.
        
        :param key: string defining the action/message
        :param *args: list of arguments
        
        The key and *args can be one of the following...
        
        In the example/sequence diagram below messages from A are sent by the one that initiates the test. 
        Messages from B represent the one that responds to the first one.

        * A: ping (used to measure latency)
        * B: pong
        """
        if key == "ping": # from A
            print "PING"
            self._send_message("pong")
            self.notify_api(self.caller, 'info', "Received ping")
        elif key == "pong": # from B 
            print "PONG"
            self.notify_api(self.caller, 'info', "Received pong")
    
    def _send_message(self, key, args_list=[]):
        """
        Sends a message to the current remote contact through the com_chan with key "network_test".
        :param args: list
        """
        try:
            # list with key as 0th element
            self.com_chan.callRemote("pinger", key, args_list)
        except AttributeError, e:
            log.error("Could not send message to remote. ComChan is None" + e.message)
        log.debug("Sent %s. %r" % (key, args_list))
        self.notify_api(self.caller, 'info', "Sent ping.")
    
    def _register_my_callbacks_to_com_chan(self, com_channel):
        """
        this is where we actually registers the callbacks
        """
        if com_channel is None:
            log.error("pinger.py: The provided com_channel is None !")
        else:
            com_channel.add(self.on_remote_message, "pinger")
            log.info("Registered the com_chan callback with a new com_channel for the pinger instance.")

def on_com_chan_connected(connection_handle, role="client"):
    """
    Called from the Connector class in its com_chan_started_client or com_chan_started_server method.

    registers the com_chan callback for network_test
    
    :param connection_handle: connectors.Connection object.
    :param role: string "client" or "server"
    We actually do not care if this miville is a com_chan client or server.
    """
    global _api
    global _pingers

    pinger = PingerTester()
    contact = connection_handle.contact
    pinger.contact = contact
    pinger.com_chan = connection_handle.com_chan
    pinger._register_my_callbacks_to_com_chan(pinger.com_chan)
    pinger.api = _api
    pinger.remote_addr = contact.address
    _pingers[pinger.contact.name] = pinger
    log.debug("pingers: " + str(_pingers))

def on_com_chan_disconnected(connection_handle):
    """
    Called when a connection is stopped
    """
    global _pingers
    try:
        del _pingers[connection_handle.contact.name]
        log.debug("pingers: " + str(_pingers))
    except Exception, e:
        log.error("error in on_com_chan_disconnected : bad key " + e.message)

def get_pinger_for_contact(contact_name=None):
    """
    Returns a NetworkTester instance for contact name, or raises a  NetworkError.

    :param contact_name: unicode string
    """
    global _pingers
    ret = None
    try:
        ret = _pingers[contact_name]
    except KeyError:
        raise KeyError("In get_pinger_for_contact: No Network pinger for contact.")
    else:
        return ret

# functions ---------------------------------------------
def start(subject):
    """
    Initial setup of the whole module for miville's use.
    
    Notifies with 'error' key  if `iperf` command not found. (CommandNotFoundError)
    Returns a NetworkTester instance with the server started.
    """
    global _api

    _api = subject
    
    connectors.register_callback("pinger_on_connect", on_com_chan_connected, event="connect")
    connectors.register_callback("pinger_on_disconnect", on_com_chan_disconnected, event="disconnect")
    #return tester

