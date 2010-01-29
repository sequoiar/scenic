#!/usr/bin/env python
# -*- coding: utf-8 -*-

from twisted.trial import unittest
from scenic import sic
from twisted.internet import reactor
from twisted.internet import defer

class Test_01_TCPClientServer(unittest.TestCase):
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
        self.satisfaction_deferred = None
        self.client_factory.connected_deferred.addCallback(self._on_client_connected)
        self.sender_proto = None
        return self.client_factory.connected_deferred

    def _on_client_connected(self, proto):
        self.sender_proto = proto
    
    def _send(self, msg):
        self.sender_proto.send_message(msg)
    
    def _on_received(self, server_protocol, msg):
        self.failUnlessEqual(msg, {"msg": "ping"})
        #print msg
        self.satisfaction_deferred.callback(True)
        return True

    def testSingleMessage(self):
        msg = {"msg": "ping"}
        self.satisfaction_deferred = defer.Deferred()
        self._send(msg)
        return self.satisfaction_deferred

    def tearDown(self):
        self.clientPort.transport.loseConnection()
        return defer.DeferredList([self.serverPort.stopListening()])

 
class Test_Client_Server(unittest.TestCase):
    def setUp(self):
        pass

    def test_client_server(self):
        pass
        
    def tearDown(self):
        pass
