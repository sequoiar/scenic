# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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


# Twisted imports
from twisted.internet import reactor
from twisted.internet import protocol

class GSTProcessProtocol(protocol.ProcessProtocol):
    def connectionMade(self):
#        print 'YES'
#            self.transport.write(self.text)
        self.transport.closeStdin()

    def outReceived(self, data):
        print 'DATA:', data
#        self.transport.loseConnection()
    
    
    
    
if __name__ == "__main__":
    exe = '/Users/etienne/Documents/propulesart/miville/trunk/public/py/protocols/ipcp_server.py'
    p = GSTProcessProtocol()
    reactor.spawnProcess(p, exe, [exe], usePTY=True)
    reactor.run()   
    
    
    