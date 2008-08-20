
from twisted.internet import reactor, defer
from twisted.web.client import getPage

import time


class ComTest(object):
    def __init__(self):
        self.deferred = defer.Deferred()

    def send(self, cmd):
        reactor.callLater(2, self.receive, cmd)
        return self.deferred
    
    def receive(self, cmd):
        self.deferred.callback(cmd*2)


def get_data(cmd):
    com = ComTest()
    return com.send(cmd)


@defer.inlineCallbacks
def test():
    print 'Start test'
    query = "http://www.google.com"
    com = ComTest()
    prev = time.time()
    print 'before getPage', prev
    content = yield com.send(query)
#    print content
    print 'after getPage', time.time() - prev
#    return content
    defer.returnValue(content)    
    print 'End test'
#test = defer.inlineCallbacks(test)

def out(t):
    print t
    reactor.stop()
        
def main():
    prev = time.time()
    print "Starting ", prev
    t = test()
    print "Stoping ", time.time() - prev
    t.addCallback(out)
#    reactor.callLater(5, out, t)
#    print t
#    reactor.stop()

reactor.callLater(0, main)
reactor.run()