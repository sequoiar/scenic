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
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.



# System imports
#from types import FunctionType
from inspect import ismethod, isfunction

# Twisted imports
from zope.interface import implements
from twisted.internet import reactor, defer
from twisted.cred import portal, checkers, credentials, error
from twisted.spread import pb
from twisted.python import failure

# App imports
from utils import log

log = log.start('debug', 1, 0, 'com_chan')

PORT = 37054

connections = None

class ComChannel(object):
    def __init__(self):
        self.remote = None
        self.methods = {}
        
    def attached(self, remote):
        log.info("Connected: %r" % remote)
        self.remote = remote

    def main(self, *args):
        if len(args) < 1:
            log.info('Receive an empty remote call')
        else:
            cmd = args[0]
            if cmd not in self.methods:
                log.debug('Could not excecute method %s. Not in methods list.' % cmd)
            else:
                log.debug("Received: " + cmd + repr(args))
                self.methods[cmd](*args[1:])
    
    def callRemote(self, *args):
        if self.remote:
            self.remote.callRemote('main', *args)
        else:
            log.debug('No remote')
            
    def add(self, cmd, name=None):
        if not name:
            name = cmd.im_class.__name__ + '.' + cmd.__name__
        if name not in self.methods:
            self.methods[name] = cmd
        else:
            log.debug('A method with this name %s is already registered.' % name)
        
    def delete(self, name):
        if ismethod(name):
            name = name.im_class.__name__ + '.' + name.__name__
        if name in self.methods:
            del self.methods[name]
        else:
            log.debug('Could not delete the method %s because is not registered.' % name)
            

class ComChanClient(pb.Referenceable, ComChannel):
    
    remote_main = ComChannel.main
    
    def connect(self, name, address, port=37054):
        self.factory = pb.PBClientFactory()
        reactor.connectTCP(address, port, self.factory)
        deferred = self.factory.login(credentials.UsernamePassword(name, "pofword"), client=self)
        deferred.addCallback(self.attached)
        return deferred

    def disconnect(self):
        self.factory.disconnect()


class ComChanRealm:
    implements(portal.IRealm)
    def requestAvatar(self, name, remote, *interfaces):
        assert pb.IPerspective in interfaces
        avatar = ComChanContact(name)
        avatar.attached(remote)
        connections[name].attached('server', avatar)
        return pb.IPerspective, avatar, lambda a = avatar:a.detached(remote)


class ComChanContact(pb.Avatar, ComChannel):
    def __init__(self, name):
        self.name = name
        ComChannel.__init__(self)
        
    perspective_main = ComChannel.main

    def detached(self, remote):
        self.remote = None


class ComChanCheck:
    """
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
            return failure.Failure(error.UnauthorizedLogin())



def start(conns, port=PORT):
    global connections
    connections = conns
    realm = ComChanRealm()
    checker = ComChanCheck()
    p = portal.Portal(realm, [checker])
    reactor.listenTCP(port, pb.PBServerFactory(p))

def connect(name, address, port=PORT):
    log.debug('Connecting: %s:%s' % (address, port))
    channel = ComChanClient()
    deferred = channel.connect(name, address, port)
    return channel, deferred

