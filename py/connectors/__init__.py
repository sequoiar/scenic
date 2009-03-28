# -*- coding: utf-8 -*-

# Sropulpof
# # Copyright (C) 2008 Société des arts technologiques (SAT)
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
"""
Please write a description here
"""

from twisted.internet import task
# App imports
from errors import ConnectorError, ConnectionError
from connectors.states import *
from utils import common, log
from protocols import com_chan

log = log.start('debug', 1, 0, 'connectors')

"""
Dict of all Connection subclasses instances.
key = contact name 
value = instance
"""
connections = {}
connectors = {}
connect_callbacks = {}
disconnect_callbacks = {}

def chk_ob():
    print "Connections: %r" % connections

"""
Uncomment those two lines to te permettera de voir continuellement quelles sont les connexions existantes
"""
#l = task.LoopingCall(chk_ob)
#l.start(2.0, False)


class Connection(object):
    """
    Base Class representing one connection between two clients.

    The connector do not last so long and die after the connection is established. 
    Later, when there are only Connection objects that last longer. 
    There is one Connection for each Contact we are connected to. 
    Connections are initiated using the ControllerApi.start_connection method.

    The Connection has a com_chan object. The com_chan has a handle to its owner using its 'owner' attribute.
    When there is a dead perspective broker, the com chan warns his owner using ********
    When this happens, we delete the com_chan and close the connection.
    The user can also close the connection by calling the ControllerApi.stop_connection method. Of course !
    
    """
    def __init__(self, contact, api):
        self.api = api
        self.com_chan = None
        self.local_name = None
        self.contact = contact
        self.remote_com_chan_port = com_chan.PORT
        contact.state = DISCONNECTED
        self.connection = None
        port = self.contact.port
        if port == None:
            port = self.api.get_default_port(self.contact.connector)
        self.address = '%s:%s' % (self.contact.address, port)
        if self.address in connections:
            #raise ConnectionError, 'Cannot connect. This address \'%s\' already have a connection (%s). %s' % (self.address, self.contact.name, str(connections))
            log.error('This address \'%s\' already have a connection (%s). %s. Connecting anyways.' % (self.address, self.contact.name, str(connections)))
        connections[self.address] = self

    def start(self):
        if self.contact.state == DISCONNECTED:
            self.contact.state = ASKING
            self._start()
        else:
            raise ConnectionError, 'A connection is already established.'

    def _start(self):
        raise NotImplementedError, '_start() method not implemented for this connector: %s.' % self.contact.connector

    def accepted(self, port=com_chan.PORT):
#        if port is None:
#            port = com_chan.PORT
        self.remote_com_chan_port = int(port)
        self._accepted()
        self.api.notify(self, {'name':self.contact.name,
                               'address':self.contact.address,
                               'msg':'The invitation has been accepted'},'answer')
        self.setup()

    def _accepted(self):
        raise NotImplementedError, '_accepted() method not implemented for this connector: %s.' % self.contact.connector

    def refused(self):
        self._refused()
        self.api.notify(self, {'name':self.contact.name,
                               'address':self.contact.address,
                               'msg':'The invitation was refused'},'answer')
        self.cleanup()

    def _refused(self):
        raise NotImplementedError, '_refused() method not implemented for this connector: %s.' % self.contact.connector

    def timeout(self):
        self.cleanup()
        self.api.notify(self, {'name':self.contact.name,
                               'address':self.contact.address,
                               'msg':'Timeout, we got no answer'},'answer')

    def connection_failed(self, err=None):
        self.cleanup()
        self.api.notify(self, {'address':self.contact.address, 
                                   'port':self.contact.port,
                                   'name':self.contact.name,
                                   'exception':'%s' % err,
                                   'msg':'Connection failed',
                                   'context':'connection'})

    def stop(self):
        if self.contact.state > DISCONNECTED and self.contact.state < DISCONNECTING:
            self.contact.state = DISCONNECTING
            self._stop()
            return 'Connection with %s stopped.' % self.contact.name
        else:
            return 'Cannot disconnect, not connected.'

    def _stop(self):
        raise NotImplementedError, '_stop() method not implemented for this connector: %s.' % self.contact.connector

    def setup(self):
        self.contact.state = CONNECTING
        channel, deferred = com_chan.connect(self.local_name, self.contact.address, self.remote_com_chan_port)
        deferred.addCallback(self.attached, channel)
        deferred.addErrback(self.not_attached)

    def attached(self, client, channel):
        """
        Callback for when we have a successful connection. 
        
        """
        self.com_chan = channel
        self.com_chan.owner = self # added for when there is an error and we need to delete the com_chan.
        self.contact.state = CONNECTED
        if client == 'server':
            self.com_chan_started_server()
        else:
            self.com_chan_started_client()

    def not_attached(self, client):
        log.error('Could not start the communication channel. Closing connection')
        self.api.notify(self, {'address':self.contact.address, 
                               'port':self.contact.port,
                               'name':self.contact.name,
                               'exception':'Could not start the communication channel. Closing the connection.',
                               'msg':'Connection failed',
                               'context':'connection'}, 'connection_failed')
        self.stop()

    def com_chan_started_client(self):
        """
        Called when the com_chan is established.
        That calls all register callbacks passing them the connection.
        """
        for callback in connect_callbacks.values():
            callback(self, 'client')
        self._com_chan_started_client()

    def com_chan_started_server(self):
        """
        Called when the com_chan is established.
        That calls all register callbacks passing them the connection.
        """
        for callback in connect_callbacks.values():
            callback(self, 'server')
        self._com_chan_started_server()

    def _com_chan_started_client(self):
        raise NotImplementedError, 'com_chan_started_client() method not implemented for this connector: %s.' % self.contact.connector

    def _com_chan_started_server(self):
        raise NotImplementedError, 'com_chan_started_server() method not implemented for this connector: %s.' % self.contact.connector

    def cleanup(self, called_by_com_chan=False):
        """
        Called when closing a connection with a contact.

        :param called_by_com_chan: boolean to prevent from infinite loops.
        """
        # TODO: just added the next line
        if called_by_com_chan:
            for callback in disconnect_callbacks.values():
                callback(self)  
        if hasattr(self.com_chan, 'disconnect') and not called_by_com_chan:
            self.com_chan.disconnect()
        if self.contact.state == DISCONNECTING:
            try:    # TODO: do this correctly
                self.api.stop_streams(self)
            except:
                pass
        self.contact.state = DISCONNECTED
        self.contact.connection = None
        try:
            self.com_chan.connection = None
        except:
            # TODO: remove this print
            print "errrreur deleting com_chan"
        try:
            del connections[self.address]
        except KeyError:
            pass

    def __del__(self):
        log.debug("__del__")

def create_connection(contact, api):
    """
    Connects to a contact using its specified connector (an attribute of the Contact class). 

    """
    if contact.kind == 'group':
        raise NotImplementedError, 'Group contact not implemented for the moment.'
    if contact.state > DISCONNECTED:
        raise ConnectionError, 'Contact \'%s\' already engage in a connection. State: %s.' % (contact.name, contact.state)
#    if contact.name in connections:
#        raise ConnectionError, 'Can not connect. This contact \'%s\' already have a connection.' % contact.name
    if not contact.connector:
        raise ConnectorError, 'Cannot connect. No connector specified for that contact.'
    if contact.connector not in connectors:
        raise ConnectorError, 'Cannot connect. Connector \'%s\' not available.' % contact.connector

    klass = getattr(connectors[contact.connector], 'Connection' + contact.connector.title())
    contact.connection = klass(contact, api)
    return contact.connection


def stop_connection(contact):
    """
    Disconnect from a contact.
    """
    if contact.state == DISCONNECTED:
        return 'Cannot stop the connection because there\'s no connection.'
    if contact.state in (DISCONNECTING, HUNGUP):
        pass
#        return 'Cannot stop the connection because it\'s already in the process of stopping.'
    if contact.state in (ASKING, CONNECTING, CONNECTED):
        if not contact.connection:
            raise ConnectionError, 'There\'s is no connection. State (%s) doesn\'t match' % contact.state
        if contact.state == ASKING:
            return contact.connection.stop_asking()
        if contact.state == CONNECTING:
            return contact.connection.stop_connecting()
        if contact.state == CONNECTED:
            return contact.connection.stop()
    return 'Cannot stop the connection. Unknown connection state (%s)' % contact.state

def receive_connection(address, port=None):
    """
    If contact is not in address book, adds it
    """
    # TODO is contact a global variable ?
    if port:
        name = '%s:%s' % (address, port)
    else:
        name = address
    if name in contacts:
        contact = contacts[name]
    else:
        contact = adb.add(name, address, port,)


def load_connectors(api):
    """
    On startup, loads all modules that are part of this package.
    """ 
    modules = common.load_modules(common.find_modules('connectors'))
    for module in modules:
        name = module.__name__.rpartition('.')[2]
        try:
            module.start(api)
        except Exception, e:
            log.error('Connector \'%s\' failed to start. %s' % (name, e))
            raise # TODO: remove me !
        else:
            connectors[name] = module
            log.info('Connector \'%s\' started.' % name)
    return connectors


def register_callback(key, callback, event="connect"):
    """
    registers a callback for "disconnect" or "connect" event.
    
    :param callback: a function of method to call. Its first argument will be the com_chan object. If it is a "connect" callback, its second argument will be the string "server" or "client"
    
    If you ever want to unregister a callback, do it manually or write an other function for it.
    """
    #TODO: check if a callback with the same key is not already register
    # (or do not use key at all?)
    if event == "disconnect":
        disconnect_callbacks[key] = callback
    else:
        connect_callbacks[key] = callback

