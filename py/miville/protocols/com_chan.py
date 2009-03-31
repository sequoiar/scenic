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
A ComChannel object is tunnel for remote procedures between a client
and a server. The client initiates the Connection to one of its Contact
instances, and then uses the Connection's ComChannnel object to communicate. 

Both server and client need to register callbacks. That is how the 
ComChannel class is used.
"""

# System imports
#from types import FunctionType
from inspect import ismethod, isfunction

# Twisted imports
from zope.interface import implements
from twisted.internet import reactor, defer
from twisted.cred import portal, checkers, credentials, error
from twisted.spread import pb
from twisted.spread.pb import DeadReferenceError
from twisted.python import failure

# App imports
from miville.utils import log

log = log.start('debug', 1, True, 'com_chan') # LOG TO FILE = True

# constants
PORT = 37054

# module's globals
# dict of Connection instances.
connections = None

class ComChannel(object):
    """
    Class for calling remote procedure calls and providing
    procedures for the remote peer.

    Server.
    """
    def __init__(self):
        self.remote = None
        self.methods = {}
        self.owner = None # The connection that owns it. In order to delete this instance. 
        
    def attached(self, remote):
        """
        Called when the Connection is made. 
        """
        log.info("Connected: %r" % remote)
        self.remote = remote

    def main(self, *args):
        """
        Called when a remote call is received. 
        
        This method with look in the registered callbacks and call the proper one.
        :param *args: is a tuple of data, whose 0th element is the name of the procedure to call.
        """
        if len(args) < 1:
            log.info('Received an empty remote call')
        else:
            cmd = args[0]
            if cmd not in self.methods:
                log.debug('Could not excecute method %s. Not in methods list.' % cmd)
            else:
                log.debug("Received: " + cmd + repr(args))
                self.methods[cmd](*args[1:])
    
    def callRemote(self, *args):
        """
        Calls a remote procedures provided by the remote peer.
        
        Warning : will close the connection with remote host in case of DeadReferenceError.
        (when unable to reach remote miville.)
        
        :param *args: is a tuple of data, whose 0th element is the name of the remote class and method in the form Class.method to call. The other elements are the args.
        For now, no object, just list, dict, str and int are supported.
        """
        if self.remote:
            try:
                self.remote.callRemote('main', *args)
            except DeadReferenceError, e:
                # This error is raised when a method is called on a dead reference (one whose
                # broker has been disconnected).
                # TODO notify observers with an error.
                # TODO : disconnect com_chan
                log.debug("DeadReferenceError in com_chan.callRemote() ! Will close the connection.")
                try:
                    self.owner.stop() # STOPS THE CONNECTION !!! 
                    # TODO: self.owner.cleanup()
                except:
                    log.error("Error calling Connection.stop()" + e.message)
                # log.debug("Connection lost in ComChannel.callRemote: " + e.message)
        else:
            log.debug('No remote')
            
    def add(self, cmd, name=None):
        """
        Registers a local callback.
        
        Makes it available to the remote peer.
        :param cmd: The callback to register.
        :param name: The name to which identify this callback. 
        (Defaults to the class.method provided as the cmd argument, but you are better to provide a name)
        """
        if not name:
            name = cmd.im_class.__name__ + '.' + cmd.__name__
        if name not in self.methods:
            self.methods[name] = cmd
        else:
            log.debug('A method with this name %s is already registered.' % name)
        
    def delete(self, name):
        """
        Deletes a local callback. 

        Makes it no longer available to be called by remote peer.
        :param name: The name to which identify this callback.
        """
        if ismethod(name):
            name = name.im_class.__name__ + '.' + name.__name__
        if name in self.methods:
            del self.methods[name]
        else:
            log.debug('Could not delete the method %s because is not registered.' % name)
            

class ComChanClient(pb.Referenceable, ComChannel):
    """
    ComChan on client side.

    Slightly different on initialization time.
    """
    remote_main = ComChannel.main
    
    def connect(self, name, address, port=37054):
        self.factory = pb.PBClientFactory()
        reactor.connectTCP(address, port, self.factory)
        deferred = self.factory.login(credentials.UsernamePassword(name, "pofword"), client=self)
        deferred.addCallback(self.attached)
        return deferred

    def disconnect(self):
        #print "COMCHAN_DISCONNECTED in ComChanClient"
        self.owner.cleanup(True) # arg = called by com_chan
        self.factory.disconnect()


class ComChanRealm:
    """
    Creates an avatar. (user)
    Attaches it a ComChannel
    """
    implements(portal.IRealm)
    def requestAvatar(self, name, remote, *interfaces):
        # global connections
        assert pb.IPerspective in interfaces
        avatar = ComChanContact(name)
        avatar.attached(remote)
        connections[name].attached('server', avatar)
        return pb.IPerspective, avatar, lambda a = avatar:a.detached(remote)


class ComChanContact(pb.Avatar, ComChannel):
    """
    The avatar. (user)
    """
    def __init__(self, name):
        self.name = name
        ComChannel.__init__(self)
        
    perspective_main = ComChannel.main

    def detached(self, remote):
        print "COMCHAN_DISCONNECTED in ComChanContact"
        self.owner.cleanup()
        self.remote = None


class ComChanCheck:
    """
    Verifies the authentification for an avatar with its credentials. 

    An extremely simple credentials checker.
    It check if the client is already in the connections pool
    and that the password is the good universal one
    (stupid password check but better then nothing).
    """
    implements(checkers.ICredentialsChecker)

    credentialInterfaces = (credentials.IUsernamePassword,
                            credentials.IUsernameHashedPassword)
    
    def requestAvatarId(self, credentials):
        name = str(credentials.username)
        log.info('\'%s\' is trying to connect.' % name)
        if name in connections:
            return defer.maybeDeferred(credentials.checkPassword,
                                       'pofword').addCallback(
                                        self._cbPasswordMatch, name)
        else:
            return defer.fail(error.UnauthorizedLogin())

    def _cbPasswordMatch(self, matched, name):
        if matched:
            return name
        else:
            log.debug('Unauthorized login')
            return failure.Failure(error.UnauthorizedLogin())

# functions 
def start(api, conns, port=PORT, interfaces=''):
    """
    Starts the module for miville's use.
    
    :param conns: list of Connection instances.
    "param port: port number.
    """
    global connections

    connections = conns
    realm = ComChanRealm()
    checker = ComChanCheck()
    p = portal.Portal(realm, [checker])
    api.listen_tcp(port, pb.PBServerFactory(p), interfaces)

def connect(name, address, port=PORT):
    """
    Use case implementation. 
    Connects to a contact
    
    :returns: channel, deferred
    """
    log.debug('Connecting: %s:%s' % (address, port))
    channel = ComChanClient()
    deferred = channel.connect(name, address, port)
    return channel, deferred

