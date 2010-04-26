#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Simple protocol using JSON over TCP.
SIC Stands for "SIP Spelled Incorrectly".
"""

# JSON import:
try:
    import json # python 2.6
except ImportError:
    import simplejson as json # python 2.4 to 2.5
try:
    _tmp = json.loads
except AttributeError:
    import warnings
    import sys
    warnings.warn("Use simplejson, not the old json module.")
    sys.modules.pop('json') # get rid of the bad json module
    import simplejson as json

from twisted.internet import reactor
from twisted.internet import protocol
from twisted.internet import defer
from twisted.protocols import basic
from scenic import sig

VERBOSE = True

class SICProtocol(basic.LineReceiver):
    """
    Receives and sends JSON lines.
    Twisted add to it a factory attribute.
    """
    delimiter = '\n'

    def connectionMade(self):
        if hasattr(self, "factory"):
            if hasattr(self.factory, 'connected_deferred'):
                if not self.factory.connected_deferred.called:
                    self.factory.connected_deferred.callback(self)
            else:
                print "SIC: No connected_deferred"
        else:
            print "SIC: No factory"

    def lineReceived(self, data):
        """
        Received JSON.
        """
        if VERBOSE:
            print "SIC: Received:", data.strip()
        try:
            d = json.loads(data.strip())
        except TypeError, e:
            print(str(e))
        except ValueError, e:
            print(str(e))
        else:
            self.factory.dict_received_signal(self, d)

    def send_message(self, d):
        """
        Sends a dict serialized in JSON.
        :param d: dict
        """
        if VERBOSE:
            print "SIC: send_message", d
        if type(d) is not dict:
            raise TypeError("A dict is needed.")
        else:
            data = json.dumps(d)
            return self.transport.write(data + "\n")
    
    def get_peer_ip(self):
        try:
            ip = self.transport.getPeer().host
            return ip
        except AttributeError:
            print "SIC: not connected"
            return None

class ClientFactory(protocol.ClientFactory):
    protocol = SICProtocol
    def __init__(self):
        self.connected_deferred = defer.Deferred()

    def clientConnectionFailed(self, connector, reason):
        print 'Connection failed. Reason:', reason
        self.connected_deferred.errback(reason)
        return True

class ServerFactory(protocol.ServerFactory):
    protocol = SICProtocol
    def __init__(self):
        self.connected_deferred = defer.Deferred()
        self.dict_received_signal = sig.Signal()

if __name__ == "__main__":
    class Dummy(object):
        def on_received(self, protocol, d):
            print "received", d
            reactor.stop()

    def on_connected(protocol):
        ret = protocol.send_message({"method": "ping"})
        print "sent ping"
        return ret

    def on_error(failure):
        print "SIC: Error trying to connect.", failure
        reactor.stop()
        
    VERBOSE = True
    PORT_NUMBER = 15555
    s = ServerFactory()
    d = Dummy()
    s.dict_received_signal.connect(d.on_received)
    reactor.listenTCP(PORT_NUMBER, s)

    client_factory = ClientFactory()
    clientPort = reactor.connectTCP("localhost", PORT_NUMBER, client_factory)
    client_factory.connected_deferred.addCallback(on_connected)
    reactor.run()
