#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.
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
from scenic import logger

log = logger.start(name="sic")

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
                log.warning("SIC: No connected_deferred")
        else:
            log.warning("SIC: No factory")

    def lineReceived(self, data):
        """
        Received JSON.
        """
        log.debug("Received %s" % (data.strip()))
        try:
            d = json.loads(data.strip())
        except TypeError, e:
            log.error(str(e))
        except ValueError, e:
            log.error(str(e))
        else:
            self.factory.dict_received_signal(self, d)

    def send_message(self, d):
        """
        Sends a dict serialized in JSON.
        :param d: dict
        """
        log.debug("SIC: send_message %s" % (d))
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
            log.error("SIC: not connected")
            return None

class ClientFactory(protocol.ClientFactory):
    protocol = SICProtocol
    def __init__(self):
        self.connected_deferred = defer.Deferred()

    def clientConnectionFailed(self, connector, reason):
        log.error('Connection failed. Reason: %s' % (reason))
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
        
    PORT_NUMBER = 15555
    s = ServerFactory()
    d = Dummy()
    s.dict_received_signal.connect(d.on_received)
    reactor.listenTCP(PORT_NUMBER, s)

    client_factory = ClientFactory()
    clientPort = reactor.connectTCP("localhost", PORT_NUMBER, client_factory)
    client_factory.connected_deferred.addCallback(on_connected)
    reactor.run()
