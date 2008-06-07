#!/usr/bin/env python

# Twisted imports
from twisted.internet import reactor, protocol
from twisted.conch import telnet

from twisted.plugin import IPlugin
from ui import iui

class Test:
    implements(IPlugin, iui.IUI)

    def __init__(self, yieldStressFactor, dielectricConstant):
        print "prout"


class GuiTelnet(telnet.Telnet):
    """This is just about the simplest possible protocol"""
    
    def dataReceived(self, data):
        "As soon as any data is received, write it back."
        self.transport.write(data)


# this only runs if the module was *not* imported
if __name__ == '__main__':
    """This runs the protocol on port 8000"""
    factory = protocol.ServerFactory()
    factory.protocol = GuiTelnet
    reactor.listenTCP(8000,factory)
    reactor.run()
