#!/usr/bin/env python
# -*- coding: utf-8 -*-

from twisted.trial import unittest
from scenic import sic
from twisted.internet import reactor
from twisted.internet import defer

class Test_01_Sic(unittest.TestCase):
    PORT_NUMBER = 15555
    
    def setUp(self):
        class Dummy(object):
            def on_received(self, protocol, d):
                #print "received", d
                return True
        
        s = sic.SICServerFactory()
        dummy = Dummy()
        s.dict_received_signal.connect(dummy.on_received)
        self._port = reactor.listenTCP(self.PORT_NUMBER, s)
        return self._port    

    def test_it(self):
        def _on_connected(protocol):
            ret = protocol.send_message({"method": "ping"})
            self.client = protocol
            #print "sent ping"
            return ret

        def _on_error(failure):
            msg = "Error trying to connect. %s" % (failure)
            #print msg
            return failure
        
        deferred = sic.create_SIC_client('localhost', self.PORT_NUMBER).addCallback(_on_connected).addErrback(_on_error)
        return deferred

    def tearDown(self):
        self.client.transport.loseConnection()
        return defer.DeferredList([self._port.stopListening()])



class Test_02_TCPClientServer(unittest.TestCase):
    """
    Test the L{osc.Sender} and L{osc.Receiver} over UDP via localhost.
    """
    timeout = 1

    def setUp(self):
        self.server_factory = sic.ServerFactory()
        self.server_factory.dict_received_signal.connect(self._on_received)
        self.serverPort = reactor.listenTCP(17778, self.server_factory)
        self.client_factory = sic.ClientFactory()
        self.clientPort = reactor.connectTCP("localhost", 17778, self.client_factory)
        self.satisfied = None
        self.client_factory.connected_deferred.addCallback(self._on_client_connected)
        self.sender_proto = None
        return self.client_factory.connected_deferred

    def _on_client_connected(self, proto):
        self.sender_proto = proto
    
    def _send(self, msg):
        self.sender_proto.send_message(msg)
    
    def _on_received(self, server_protocol, msg):
        self.failUnlessEqual(msg, {"msg": "ping"})
        self.satisfied.callback(True)
        return True

    def testSingleMessage(self):
        msg = {"msg": "ping"}
        self.satisfied = defer.Deferred()
        self._send(msg)
        return self.satisfied

    def tearDown(self):
        self.clientPort.transport.loseConnection()
        return defer.DeferredList([self.serverPort.stopListening()])

 
class Test_Client_Server(unittest.TestCase):
    def setUp(self):
        pass
        
    def tearDown(self):
        pass
