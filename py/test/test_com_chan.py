# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Soci�t� des arts technologiques (SAT)
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

########### !!!!!!!! Problem closing properly the pb.PBServerFactory !!!!!!#################
import os
import sys
from twisted.trial import unittest
from twisted.internet import reactor, defer, task
from twisted.spread import pb

from connections.com_chan import ComChannel
g = False
#class for test
def send(remote):
    print 'send'
    remote.callRemote()

def send2(remote):
    print 'send2'
    remote.callRemote('Dummy.test', 'allo', 22)

def send3(remote):
    global g
    print 'send3'
    remote.callRemote('Dummy.close')
    g = True
    
############# Loop Until ################
def loopUntil(predicate, interval=0):
    d = defer.Deferred()
    
    def check():
        res = predicate()
        if res:
            d.callback(res)
    call = task.LoopingCall(check)
    
    def stop(result):
        call.stop()
        return result
    d.addCallback(stop)
    d2 = call.start(interval)
    d2.addErrback(d.errback)
    return d
#########################################
       
class Dummy(object):
    def test(self, *args):
        print args
        
    def close(self):
        global g
        print 'Closing...'
        g = True
  
            
class TestComChan(unittest.TestCase):
    def setUp(self):
        
        self.channel = ComChannel()
        self.dummy = Dummy()
        self.server_factory = pb.PBServerFactory(self.channel)
        self.listen = reactor.listenTCP(8800, self.server_factory)
        
        self.factory = pb.PBClientFactory()
        self.connect = reactor.connectTCP("localhost", 8800, self.factory)
        self.channel2 = ComChannel()
        
        d1 = self.factory.getRootObject()
          
        def gotRoot(ref):
            self.ref = ref   
            return ref
        d1.addCallback(self.channel2.remote_init)
        d1.addCallback(gotRoot)
        return defer.gatherResults([d1])


    def tearDown(self):
        #self.listen.factory.protocol.connectionLost()
        del self.channel2
        self.connect.disconnect()
        self.ref.broker.transport.loseConnection()
        
        del self.channel
        del self.dummy
        return self.listen.stopListening()

   
    def test_add(self):
        self.channel.add(self.dummy.test)
        self.channel.add(self.dummy.close)
        if (self.channel.methods['Dummy.test'] is None):
            self.fail("problem adding a method")
            
        if (self.channel.methods['Dummy.close'] is None):
            self.fail("problem adding a method")   
            

    def test_delete(self):
        self.channel.add(self.dummy.test)
        self.channel.delete('Dummy.test')
        if (len(self.channel.methods) > 0):
            self.fail("problem deleting a method")
    
    
    def test_send(self):
        global g
        #server
        self.channel.add(self.dummy.test)
        self.channel.add(self.dummy.close)
        
        # simulate a call to a root method
        reactor.callLater(0.1, send, self.channel)
        # simulate a call to a method of another class 
        reactor.callLater(0.3, send2, self.channel)
        
        # simulate a call to a root method
        reactor.callLater(0.2, send, self.channel2)
        # simulate a call to a method of another class 
        reactor.callLater(0.5, send2, self.channel2)
        # simulate a call to a method of another class 
        reactor.callLater(0.9, send3, self.channel2)

        d = defer.gatherResults([loopUntil(lambda: g == True)])
        return d
        
