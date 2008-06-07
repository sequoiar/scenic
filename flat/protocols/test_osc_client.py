#!/usr/bin/env python

# twisted imports
from twisted.internet import reactor, protocol

# OSC imports
import osc_protocol


def client():
    """Client example"""
    message = osc_protocol.OscMessage()
    message.setAddress('/allo')
    message.append('grrr')
    message.append(34)
    c.send_message('127.0.0.1', 22223, message)
    reactor.stop()


if __name__ == "__main__":
    # Client example
    c = osc_protocol.Osc()
    reactor.listenUDP(0, c)
    reactor.callLater(1, client)
    reactor.run()
