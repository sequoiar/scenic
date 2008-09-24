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
from twisted.internet import reactor
from twisted.spread import pb

# App imports
from utils import log

log = log.start('debug', 1, 0, 'com_chan')


class ComChannel(pb.Root):
    def __init__(self):
        self.remote = None
        self.methods = {}
        
    def remote_init(self, remote):
        if not self.remote:
            log.info("Received a called for %r." % remote)
            self.remote = remote
            remote.callRemote('init', self)
        
    def remote_main(self, *args):
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


def listen(port):
    channel = ComChannel()
    reactor.listenTCP(port, pb.PBServerFactory(channel))
    return channel


factory = pb.PBClientFactory()

def connect(address, port):
    reactor.connectTCP(address, port, factory)
    channel = ComChannel()
    deferred = factory.getRootObject()
    deferred.addCallback(channel.remote_init)
    return channel
    


if __name__ == '__main__':

    import sys
    
    from twisted.internet import reactor
    
    class Dummy(object):
        def test(self, *args):
            print args
            
        def close(self):
            print 'Closing...'
            reactor.callLater(5, reactor.stop)
            
    def send(remote):
        remote.callRemote()
    
    def send2(remote):
        remote.callRemote('Dummy.test', 'allo', 22)

    def send3(remote):
        remote.callRemote('Dummy.close')
    
    port = 8800
    
    if len(sys.argv) > 1:
        # it's a server
        # Do this when connecting (SIP)
        channel = ComChannel()
        dummy = Dummy()
        channel.add(dummy.test)
        channel.add(dummy.close)
        channel.delete('test')
        print channel.methods
        reactor.listenTCP(port, pb.PBServerFactory(channel))
        
        # simulate a call to a root method
        reactor.callLater(3, send, channel)

        # simulate a call to a method of another class 
        reactor.callLater(8, send2, channel)

    else:
        # it's client
        # Do this when after a connection (SIP) is establish
        factory = pb.PBClientFactory()
        reactor.connectTCP("localhost", port, factory)
        channel = ComChannel()
        deferred = factory.getRootObject()
        deferred.addCallback(channel.remote_init)
        
        # simulate a call to a root method
        reactor.callLater(5, send, channel)
    
        # simulate a call to a method of another class 
        reactor.callLater(7, send2, channel)
    
        # simulate a call to a method of another class 
        reactor.callLater(9, send3, channel)
        reactor.callLater(12, reactor.stop)
    
    reactor.run()
