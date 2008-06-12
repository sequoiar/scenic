#!/usr/bin/env python

# Twisted imports
from zope.interface import implements
from twisted.internet import reactor, protocol
from twisted.conch import telnet

import ui

class CliUI:
#    implements(ui.iui.IUI)

    def __init__(self):
        print "prout"
    
    def parse(self, data):
        print data.strip()


class TelnetServer(telnet.Telnet):
    """A Telnet server to control the application from a command line interface."""

#    def __init__(self):
#        self.ui = CliUI
        
    def connectionMade(self):
        print "Someone connecting from Telnet"
        self.transport.write("Welcome to Sropulpof!\n")
        self.transport.write("pof: ")

    def dataReceived(self, data):
        if data.strip().lower() in ("exit", "quit"):
            print "Telnet connection stopped"
            self.transport.write("Good Bye\n")
            self.transport.loseConnection()
        else:
            self.transport.write("pof: ")
#           self.ui.parse(data)


def start():
    """This runs the protocol on port 8000"""
    factory = protocol.ServerFactory()
    factory.protocol = TelnetServer
    reactor.listenTCP(8000, factory)
    

#start()

# this only runs if the module was *not* imported
if __name__ == '__main__':
    reactor.run()
