#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Unit tests for the SIC protocol, sender and receiver.
"""

from twisted.trial import unittest
from scenic import sic
from scenic import communication
from twisted.internet import reactor
from twisted.internet import defer

class Test_01_SIC_Client_Server(unittest.TestCase):
    """
    Tests the Server and Receiver
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
        return defer.DeferredList([self.client_factory.connected_deferred, self.server_factory.connected_deferred])

    def _on_client_connected(self, proto):
        self.sender_proto = proto

    def _send(self, msg):
        self.sender_proto.send_message(msg)

    def _on_received(self, server_protocol, msg):
        self.failUnlessEqual(msg, {"msg": "ping"})
        #print msg
        ip = server_protocol.get_peer_ip()
        self.satisfaction_deferred.callback(True)
        return True

    def test_single_message(self):
        msg = {"msg": "ping"}
        self.satisfaction_deferred = defer.Deferred()
        self._send(msg)
        return self.satisfaction_deferred

    def tearDown(self):
        self.clientPort.transport.loseConnection()
        return defer.DeferredList([self.serverPort.stopListening()])

class DummyApp(object):
    def on_server_receive_command(self, proto, d):
        pass
    def on_client_rcv_command(self, client, msg):
        #print "client received:", msg
        pass
    def on_client_socket_timeout(self, client):
        pass
    def on_connection_error(self, client, err, msg):
        pass
    def on_client_connecting(self, client):
        pass
    def client_answer_timeout(self, client):
        pass

class Test_02_Scenic_Client_Server(unittest.TestCase):
    def setUp(self):
        PORT = 17474
        app = DummyApp()
        self.server = communication.Server(app, PORT)
        self.server.received_command_signal.connect(self._on_received_command)
        self.client = communication.Client()
        self.client.connection_error_signal.connect(app.on_connection_error)
        d1 = self.server.start_listening()
        d2 = self.client.connect("localhost", PORT)
        #print "starting server...."
        self.recv_deferred = None
        return defer.DeferredList([d1, d2])

    def _on_received_command(self, msg, addr):
        self.failUnlessEqual(msg, {"msg":"ping"})
        self.recv_deferred.callback(True)
        return True

    def test_single_message(self):
        #def _sent(result):
        #    return result
        self.recv_deferred = defer.Deferred()
        self.client.send({"msg": "ping"})
        #deferred.addCallback(_sent)
        #return deferred

    def tearDown(self):
        d1 = self.server.close()
        d2 = self.client.disconnect()
        #print d2
        return defer.DeferredList([d1, d2])
