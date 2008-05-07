"""Client.py: uses the calculation service across the network"""

from twisted.spread import pb
from twisted.internet import reactor

PORT = 8992
COUNT = 3

class ModelCalculator:
    def __init__(self, host):
        self.a = 42
        self.b = 47
        self.count = 0
        self.total = 0
        print 'Connecting to servers..'
        for port in range(PORT, PORT + COUNT):
            factory = pb.PBClientFactory()
            reactor.connectTCP(host, port, factory)
            factory.getRootObject()\
                .addCallbacks(self.connected, self.failure)

    def connected(self, perspective):
        perspective.callRemote('calculate', self.a, self.b)\
            .addCallbacks(self.success, self.failure)
        print "connected"
        self.a += 10
        self.b += 11

    def success(self, result):
        print result
        self.total += result[0]
        self.count += 1 # Alternative: use a deferred list
        if (self.count == COUNT):
            print "total:", self.total
            reactor.stop()
      
    def failure(self, _):
        print "remote failure"
        reactor.stop()

ModelCalculator("127.0.0.1")
reactor.run()

