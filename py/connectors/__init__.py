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

from twisted.internet import task
# App imports
from errors import ConnectorError, ConnectionError
from connectors.states import *
from utils import common, log
from protocols import com_chan


log = log.start('debug', 1, 0, 'connectors')

connections = {}
connectors = {}

def chk_ob():
    print "Connections: %r" % connections

#l = task.LoopingCall(chk_ob)
#l.start(2.0, False)


class Connection(object):
    """
    Base Class representing one connection between two clients.
    """
    def __init__(self, contact, api):
        self.api = api
        self.com_chan = None
        self.local_name = None
        self.contact = contact
        contact.state = DISCONNECTED
        self.connection = None
        port = self.contact.port
        if port == None:
            port = self.api.get_default_port(self.contact.connector)
        self.address = '%s:%s' % (self.contact.address, port)
        if self.address in connections:
            raise ConnectionError, 'Cannot connect. This address \'%s\' already have a connection (%s).' % (self.address, self.contact.name)
        connections[self.address] = self

    def start(self):
        if self.contact.state == DISCONNECTED:
            self.contact.state = ASKING
            self._start()
        else:
            raise ConnectionError, 'A connection is already established.'

    def _start(self):
        raise NotImplementedError, '_start() method not implemented for this connector: %s.' % self.contact.connector

    def accepted(self):
        self._accepted()
        self.api.notify(self, 'The invitation to %s (%s) was accepted.' % (self.contact.name, self.contact.address), 'answer')
        self.setup()

    def _accepted(self):
        raise NotImplementedError, '_accepted() method not implemented for this connector: %s.' % self.contact.connector

    def refused(self):
        self._refused()
        self.api.notify(self, 'The invitation to %s (%s) was refused.' % (self.contact.name, self.contact.address), 'answer')
        self.cleanup()

    def _refused(self):
        raise NotImplementedError, '_refused() method not implemented for this connector: %s.' % self.contact.connector

    def timeout(self):
        self.cleanup()
        self.api.notify(self, 'Timeout, the other side didn\'t respond soon enough.', 'answer')

    def connection_failed(self, err=None):
        self.cleanup()
        if err:
            self.api.notify(self,
                        'Connection failed. Address: %s | Port: %s | Error: %s' % (self.contact.address, self.contact.port, err),
                        'info')
        else:
            self.api.notify(self,
                        'Connection failed. Address: %s | Port: %s' % (self.contact.address, self.contact.port),
                        'info')

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
        channel, deferred = com_chan.connect(self.local_name, self.contact.address)
        deferred.addCallback(self.attached, channel)
        deferred.addErrback(self.not_attached)

    def attached(self, client, channel):
        self.com_chan = channel
        if client == 'server':
            self.com_chan_started_server()
        else:
            self.com_chan_started_client()

    def not_attached(self, client):
        log.error('Could not start the communication channel. Closing connection')
        self.api.notify(self,
                    'Connection failed. Address: %s | Port: %s' % (self.contact.address, self.contact.port),
                    'info')
        self.stop()

    def com_chan_started_client(self):
        raise NotImplementedError, 'com_chan_started_client() method not implemented for this connector: %s.' % self.contact.connector

    def com_chan_started_server(self):
        raise NotImplementedError, 'com_chan_started_server() method not implemented for this connector: %s.' % self.contact.connector

    def cleanup(self):
        if hasattr(self.com_chan, 'disconnect'):
            self.com_chan.disconnect()
        if self.contact.state == DISCONNECTING:
            self.api.stop_streams(self)
        self.contact.state = DISCONNECTED
        self.contact.connection = None
        del connections[self.address]


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
    
    It returned to the core as a dict. The keys are the name of each connector module.
    """
    modules = common.load_modules(common.find_modules('connectors'))
    for module in modules:
        name = module.__name__.rpartition('.')[2]
        try:
            module.start(api)
        except:
            log.error('Connector \'%s\' failed to start.' % name)
        else:
            connectors[name] = module
            log.info('Connector \'%s\' started.' % name)
    return connectors

