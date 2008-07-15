#!/usr/bin/env python

# twisted imports
from twisted.internet import reactor, protocol

# OSC imports
import osc_protocol


def test(pattern, tags, data, addr):
    print pattern
    print tags
    print data
    print addr
    message = osc_protocol.OscMessage()
    message.setAddress('/allo')
    message.append('grrr')
    s.send_message(addr[0], addr[1], message)
#    return message



if __name__ == "__main__":
    # Server example but can be both at the same time
    s = osc_protocol.Osc()
    t = reactor.listenUDP(22223, s)
    s.add_msg_handler('/allo', test)
    reactor.run()
