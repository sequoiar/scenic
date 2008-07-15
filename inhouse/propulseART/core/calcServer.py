"""Provides a calculation service across the network"""

from twisted.spread import pb
from twisted.internet import reactor

import time



PORT = 8992
COUNT = 3

class Calculator(pb.Root):
    def __init__(self, id):
        self.id = id
        print "Calculator", self.id, "running"
    def remote_calculate(self, a, b):
        print "Calculator", self.id, "calculating..."
        # fake a long computation
        time.sleep(1)
        return a + b, self.id

        
for i, port in enumerate(range(PORT, PORT + COUNT)):
    print "port:", port
    reactor.listenTCP(port, pb.PBServerFactory(Calculator(i)))

reactor.run()
