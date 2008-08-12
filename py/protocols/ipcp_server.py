#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.

# twisted imports
from twisted.internet import reactor, protocol
from twisted.protocols.basic import LineReceiver


class IPCP(LineReceiver):
    
    def lineReceived(self, line):
        print "Client: " + line

    def connectionMade(self):
        print "A client is connecting!"
        self.sendLine(r'You "are conn\"ected!" 56 45.33')
        
        
if __name__ == "__main__":
    
    # Server example
    factory = protocol.ServerFactory()
    factory.protocol = IPCP
    reactor.listenTCP(22222, factory)
    reactor.run()
