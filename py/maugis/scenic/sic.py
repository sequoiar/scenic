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
from twisted.protocols import basic

VERBOSE = False

class SICProtocol(basic.LineReceiver):
    """
    Receives and sends JSON lines.
    """
    #def connectionMade(self):
    #    print "connection made", self.transport# , self.factory
    delimiter = '\n'
    SELECTOR = "method"

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
            self.on_dict_received(d)
    
    def on_dict_received(self, d):
        print "received", d
        try:
            selector = d[self.SELECTOR]
        except KeyError, e:
            print("Dict has not key %s" % (self.SELECTOR))
        else:
            if self.factory.callbacks.has_key(selector):
                if VERBOSE:
                    print "Calling :", selector, d
                try:
                    self.factory.callbacks[selector](self, d)
                except TypeError, e:
                    print "lineReceived():", e.message
            else:
                print "Invalid selector %s." % (selector)

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
            if not d.has_key(self.SELECTOR):
                raise RuntimeError("The key %s is needed" % (self.SELECTOR))
            data = json.dumps(d)
            self.transport.write(data + "\n")

class SICServerFactory(protocol.Factory):
    """
    Factory for SIC receivers.
    
    You should attach SIC methods callbacks to an instance of this.
    """
    protocol = SICProtocol
    
    def __init__(self):
        self.callbacks = {}

    def register_handler(self, method, callback):
        if type(callback) not in (types.FunctionType, types.MethodType):
            raise TypeError("Callback '%s' is not callable" % repr(callback))    
        self.callbacks[method] = callback
    
def create_SIC_client(host, port, use_tcp=True):
    """
    Creates a SIC sender.

    When connected, will call its callbacks with the sender instance.
    :return: deferred instance
    """
    if use_tcp:
        deferred = protocol.ClientCreator(reactor, SICProtocol).connectTCP(host, port)
    else:
        deferred = protocol.ClientCreator(reactor, SICProtocol).connectUDP(host, port)
    return deferred

if __name__ == "__main__":
    def on_ping(protocol, d):
        print "received ping", d
        reactor.stop()

    def on_connected(protocol):
        protocol.send_message({"method": "ping"})
        print "sent ping"

    def on_error(failure):
        print "Error trying to connect.", failure
        reactor.stop()
        
    VERBOSE = True
    PORT_NUMBER = 15555
    s = SICServerFactory()
    s.register_handler("ping", on_ping)
    reactor.listenTCP(PORT_NUMBER, s)
    create_SIC_client('localhost', PORT_NUMBER).addCallback(on_connected).addErrback(on_error)
    reactor.run()
