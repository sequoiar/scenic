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
# along with Sropulpof. If not, see <http://www.gnu.org/licenses/>.

"""
This module manages all the com_chan dialogs in the miville application. 

How to use:
from protocols import com_chan_dialogs
"""
import connectors 

_api = None

# dict of class names with their names as keys
dialog_classes = []

# bidimensional dict whose keys are class names, and values 
# a dict with contact name as key, and instance as values.
dialog_instances = {}

class DialogError(Exception):
    """
    Error that occurs when a dialog is not found, or so.
    """
    pass
 
class ComChanDialog(object):
    """
    Base class for com_chan dialogs
    """
    def __init__(self, **kwargs):
        self.api = None
        self.com_chan = None
        self.remote_addr = None
        #self.__dict__.update(kwargs)
    
    def send_message(self, key, args):
        pass
 
    def on_remote_message(self, key, args):
        raise NotImplementedError("must be overriden in child classes")


def add_dialog(klass):
    """
    Adds a subclass of the ComChanDialog to the dict of dialogs.
    """
    global dialogs_classes
    dialog_classes[klass.__name__] = klass
    dialog_instances[klass.__name__] = {}

def start(api):
    """
    Called from the API/Core on application startup.
    """
    global _api
    _api = api
    add_dialog(StreamStart)
    add_dialog(Pinger)
    connectors.register_callback("dialogs_on_connected", on_connected, event="connect")
    connectors.register_callback("dialogs_on_disconnected", on_disconnected, event="disconnect")

def get_dialog_for_contact(caller, klass, contact_name):
    """
    Returns a com_chan dialog instance for the contact if there is one.
    
    Returns None if there is none, and notifies the api if so.
    """
    global dialogs_classes
    global dialogs_instances
    if klass not in dialog_classes:
        raise DialogError("Dialog class not in list of classes.")
    return dialog_instances[klass.__name__][contact_name] = obj

def on_disconnected(connection_handle):
    """
    Called when a com_chan is disconnected
    """
    global dialogs_instances
    dialog_instances = {}

def on_connected(connection_handle, role="client"):
    """
    Called when a new com_chan connection is established.
    
    Called from the Connector class in its com_chan_started_client or com_chan_started_server method.
 
    registers the com_chan callbacks for every dialogs
 
    :param connection_handle: connectors.Connection object.
    :param role: string "client" or "server"
    We actually do not care if this miville is a com_chan client or server.
    """
    global dialogs_classes
    global dialogs_instances
    contact = connection_handle.contact
    com_chan = connection_handle.com_chan
    remote_addr = connection_handle.contact.address

    for klass in dialog_classes:
        obj = klass()
        
        obj.api = _api
        obj.com_chan = None
        obj.remote_addr = None
        obj.contact = contact
        
        dialog_instances[klass.__name__][contact.name] = obj

def get_comchan_for_contact(caller, contact_name=None):
    """
    Returns a com_chan for the specified contact. 

    Returns None if there is none, and notifies the api if so.
    """
    pass
# ------------------------------ EXAMPLE ----------------------
# class Pinger(ComChanProtocol):
#     """
#     Example using the com_chan
#     """
#     def send_message(self, key, args):
#         pass
#  
#     def on_remote_message(self, key, args):
#         pass
#  
# add_dialog(Pinger)
#  
