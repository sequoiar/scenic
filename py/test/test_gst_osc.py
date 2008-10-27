#!/usr/bin/env python

import sys

# twisted imports
from twisted.internet import reactor, protocol

sys.path.append("protocols")

# OSC imports
import osc_protocol


def client():
    """Client example"""
    message = osc_protocol.OscMessage()
    message.setAddress('/gst')
    message.append('init')
    message.append(34)
    c.send_message('127.0.0.1', 7770, message)
    reactor.callLater(1, client2)


def client2():
    """Client example"""
    message = osc_protocol.OscMessage()
    message.setAddress('/gst')
    message.append('start')
    message.append(334)
    c.send_message('127.0.0.1', 7770, message)
    reactor.callLater(5, client3)

def cquit():
    """Client example"""
    message = osc_protocol.OscMessage()
    message.setAddress('/quit')
    c.send_message('127.0.0.1', 7770, message)
    reactor.stop()

def client3():
    """Client example"""
    message = osc_protocol.OscMessage()
    message.setAddress('/gst')
    message.append('stop')
    message.append(334)
    c.send_message('127.0.0.1', 7770, message)
    reactor.callLater(1, client2)
    reactor.callLater(10, cquit);

if __name__ == "__main__":
    # Client example
    c = osc_protocol.Osc()
    t = reactor.listenUDP(7771, c)
    print t.getHost()
    reactor.callLater(1, client)
    reactor.run()
