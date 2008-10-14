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
import os, pty, resource, signal, time, select

class GSTProcessProtocol(protocol.ProcessProtocol):
    def connectionMade(self):
#        print 'YES'
#            self.transport.write(self.text)
        self.transport.closeStdin()

    def outReceived(self, data):
        print 'DATA:', data
#        self.transport.loseConnection()
    
def start_process(exe):
        pid, fd = pty.fork()
        if pid == 0:
            max_fd = resource.getrlimit(resource.RLIMIT_NOFILE)[0]
            for i in range(3, max_fd):
                try:
                    os.close(i)
                except OSError:
                    pass
            signal.signal(signal.SIGHUP, signal.SIG_IGN)
    
    #            if self.env is None:
            os.execv(exe, [])
#        reactor.callLater(0,001, do_read, fd)
        do_read(fd)
#        while 1:

def do_read(fd):
#    time.sleep(0.1)
    if select.select([fd], [], [], 0)[0]:
        print os.read(fd, 1000)
    else:
        print 'allo'
#    do_read(fd)   
    reactor.callLater(0.001, do_read, fd)
    
    
    
if __name__ == "__main__":
    exe = '/home/etienne/workspace/miville/trunk/public/py/protocols/ipcp_server.py'
    reactor.callLater(0, start_process, exe)

#    p = GSTProcessProtocol()
#    reactor.spawnProcess(p, exe, [exe], usePTY=True)
    reactor.run()   

    
    