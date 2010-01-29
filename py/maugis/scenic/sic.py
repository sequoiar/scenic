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
    warnings.warn("Use simplejson, not the old json module.")
    sys.modules.pop('json') # get rid of the bad json module
    import simplejson as json

import types
from twisted.internet import reactor
from twisted.internet import protocol
from twisted.internet import defer
from twisted.protocols import basic

from scenic import sig

VERBOSE = False

class SICProtocol(basic.LineReceiver):
    """
    Receives and sends JSON lines.
    Twisted add to it a factory attribute.
    """
    delimiter = '\n'

    def connectionMade(self):
        if hasattr(self, "factory"):
            if hasattr(self.factory, 'connected_deferred'):
                self.factory.connected_deferred.callback(self)

    def lineReceived(self, data):
        """
        Received JSON.
        """
        if VERBOSE:
            print "Received:", data
        try:
            d = json.loads(data)
        except TypeError, e:
            print(str(e))
        else:
            self.factory.dict_received_signal(self, d)

    def send_message(self, d):
        """
        Sends a dict serialized in JSON.
        :param d: dict
        """
        if VERBOSE:
            print "send_message", d
        if type(d) is not dict:
            raise TypeError("A dict is needed.")
        else:
            data = json.dumps(d)
            return self.transport.write(data + "\n")

class SICServerFactory(protocol.Factory):
    """
    Factory for SIC receivers.
    
    You should attach SIC methods callbacks to an instance of this.
    """
    protocol = SICProtocol
    
    def __init__(self):
        self.dict_received_signal = sig.Signal()
    
def create_SIC_client(host, port):
    """
    Creates a SIC sender.

    When connected, will call its callbacks with the sender instance.
    :return: deferred instance
    """
    deferred = protocol.ClientCreator(reactor, SICProtocol).connectTCP(host, port)
    return deferred


# New stuff:
class ClientFactory(protocol.ClientFactory):
    protocol = SICProtocol
    def __init__(self):
        self.connected_deferred = defer.Deferred()

class ServerFactory(protocol.ServerFactory):
    protocol = SICProtocol
    def __init__(self):
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
        print "Error trying to connect.", failure
        reactor.stop()
        
    VERBOSE = True
    PORT_NUMBER = 15555
    s = SICServerFactory()
    d = Dummy()
    s.dict_received_signal.connect(d.on_received)
    reactor.listenTCP(PORT_NUMBER, s)
    create_SIC_client('localhost', PORT_NUMBER).addCallback(on_connected).addErrback(on_error)
    reactor.run()
